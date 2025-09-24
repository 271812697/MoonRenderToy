#pragma once
#include <filaflat/backend/DriverEnums.h>

#include <unordered_map>
#include <vector>
#include <assert.h>


namespace filament {

class BufferInterfaceBlock {
public:
    struct InterfaceBlockEntry {
        std::string name;
        uint32_t size;
        backend::UniformType type;
        backend::Precision precision;
        uint8_t associatedSampler = 0;
       
        std::string structName;
        uint32_t stride;
        std::string sizeName;
        
        InterfaceBlockEntry() = default;
        InterfaceBlockEntry(std::string name, uint32_t size, backend::UniformType type,
                backend::Precision precision = {},
                std::string structName = {},
                uint32_t stride = {}, std::string sizeName = {}) noexcept
                : name(name), size(size), type(type), precision(precision),
                associatedSampler(0),
                structName(structName), stride(stride), sizeName(sizeName) {}
        InterfaceBlockEntry(std::string name, uint8_t associatedSampler, uint32_t size, backend::UniformType type,
                backend::Precision precision = {},
                std::string structName = {},
                uint32_t stride = {}, std::string sizeName = {}) noexcept
                : name(name), size(size), type(type), precision(precision),
                associatedSampler(associatedSampler), 
                structName(structName), stride(stride), sizeName(sizeName) {}
    };

    BufferInterfaceBlock();

    BufferInterfaceBlock(const BufferInterfaceBlock& rhs) = delete;
    BufferInterfaceBlock& operator=(const BufferInterfaceBlock& rhs) = delete;

    BufferInterfaceBlock(BufferInterfaceBlock&& rhs) noexcept;
    BufferInterfaceBlock& operator=(BufferInterfaceBlock&& rhs) noexcept;

    ~BufferInterfaceBlock() noexcept;

    using Type = backend::UniformType;
    using Precision = backend::Precision;

    struct FieldInfo {
        std::string name;        // name of this field
        uint16_t offset;            // offset in "uint32_t" of this field in the buffer
        uint8_t stride;             // stride in "uint32_t" to the next element
        Type type;                  // type of this field
        bool isArray;               // true if the field is an array
        uint32_t size;              // size of the array in elements, or 0 if not an array
        Precision precision;        // precision of this field
        uint8_t associatedSampler;   // sampler associated with this field
       
        std::string structName;  // name of this field structure if type is STRUCT
        std::string sizeName;   // name of the size parameter in the shader
        // returns offset in bytes of this field (at index if an array)
        inline size_t getBufferOffset(size_t index = 0) const {
            assert(index < std::max(1u, size));
            return (offset + stride * index) * sizeof(uint32_t);
        }
    };

    enum class Alignment : uint8_t {
        std140,
        std430
    };

    enum class Target : uint8_t  {
        UNIFORM,
        SSBO
    };

    enum class Qualifier : uint8_t {
        COHERENT  = 0x01,
        WRITEONLY = 0x02,
        READONLY  = 0x04,
        VOLATILE  = 0x08,
        RESTRICT  = 0x10
    };

    class Builder {
    public:
        Builder() noexcept;
        ~Builder() noexcept;

        Builder(Builder const& rhs) = default;
        Builder(Builder&& rhs) noexcept = default;
        Builder& operator=(Builder const& rhs) = default;
        Builder& operator=(Builder&& rhs) noexcept = default;

        // Give a name to this buffer interface block
        Builder& name(std::string interfaceBlockName);

        // Buffer target
        Builder& target(Target target);

        // build and return the BufferInterfaceBlock
        Builder& alignment(Alignment alignment);

        // add a qualifier
        Builder& qualifier(Qualifier qualifier);

        // a list of this buffer's fields
        Builder& add(std::initializer_list<InterfaceBlockEntry> list);

        // add a variable-sized array. must be the last entry.
        Builder& addVariableSizedArray(InterfaceBlockEntry const& item);

        BufferInterfaceBlock build();

        bool hasVariableSizeArray() const;

    private:
        friend class BufferInterfaceBlock;
        std::string mName;
        std::vector<FieldInfo> mEntries;
        Alignment mAlignment = Alignment::std140;
        Target mTarget = Target::UNIFORM;
        uint8_t mQualifiers = 0;
        bool mHasVariableSizeArray = false;
    };

    // name of this BufferInterfaceBlock interface block
    std::string getName() const noexcept { return { mName.data(), mName.size() }; }

    // size needed for the buffer in bytes
    size_t getSize() const noexcept { return mSize; }

    // list of information records for each field
    std::vector<FieldInfo> const& getFieldInfoList() const noexcept {
        return mFieldInfoList;
    }


    size_t getFieldOffset(std::string name, size_t index)const;

    // returns offset in bytes of the transform matrix for the given external texture binding
    // returns -1 if the field doesn't exist
    size_t getTransformFieldOffset(uint8_t binding) const;

    FieldInfo const* getFieldInfo(std::string name) const;

    bool hasField(std::string name) const noexcept {
        return mInfoMap.find(name) != mInfoMap.end();
    }

    bool isEmpty() const noexcept { return mFieldInfoList.empty(); }

  

    Alignment getAlignment() const noexcept { return mAlignment; }

    Target getTarget() const noexcept { return mTarget; }

    uint8_t getQualifier() const noexcept { return mQualifiers; }

private:
    friend class Builder;

    explicit BufferInterfaceBlock(Builder const& builder) noexcept;

    static uint8_t baseAlignmentForType(Type type) noexcept;
    static uint8_t strideForType(Type type, uint32_t stride) noexcept;

    std::string mName;
    std::vector<FieldInfo> mFieldInfoList;
    std::unordered_map<std::string , uint32_t> mInfoMap;
    std::unordered_map<uint8_t, uint32_t> mTransformOffsetMap;
    uint32_t mSize = 0; // size in bytes rounded to multiple of 4
    Alignment mAlignment = Alignment::std140;
    Target mTarget = Target::UNIFORM;
    uint8_t mQualifiers = 0;
};

} 