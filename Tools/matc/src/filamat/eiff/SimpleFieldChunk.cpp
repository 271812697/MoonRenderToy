#include "SimpleFieldChunk.h"

namespace filamat {

template<>
void SimpleFieldChunk<uint8_t>::flatten(Flattener &f) {
    f.writeUint8(t);
}

template<>
 void SimpleFieldChunk<uint32_t>::flatten(Flattener &f) {
    f.writeUint32(t);
}

template<>
 void SimpleFieldChunk<uint64_t>::flatten(Flattener &f) {
    f.writeUint64(t);
}

template<>
void SimpleFieldChunk<bool>::flatten(Flattener &f) {
    f.writeBool(t);
}

template<>
 void SimpleFieldChunk<const char*>::flatten(Flattener &f) {
    f.writeString(t);
}

template<>
void SimpleFieldChunk<float>::flatten(Flattener &f) {
    f.writeUint32(reinterpret_cast<uint32_t&>(t));
}

} // namespace filamat
