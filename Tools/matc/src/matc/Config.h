#pragma once

#include <filaflat/MaterialEnums.h>
#include <filaflat/backend/DriverEnums.h>

#include <filamat/MaterialBuilder.h>

#include <map>
#include <memory>
#include <ostream>



namespace matc {

	class Config {
	public:
		enum class OutputFormat {
			BLOB,
			C_HEADER,
		};


		using Optimization = filamat::MaterialBuilder::Optimization;

		// For defines, template, and material parameters, we use an ordered map with a transparent comparator.
		// Even though the key is stored using std::string, this allows you to make lookups using
		// std::string_view. There is no need to construct a std::string object just to make a lookup.
		using StringReplacementMap = std::map<std::string, std::string, std::less<>>;

		enum class Metadata {
			NONE,
			PARAMETERS
		};

		virtual ~Config() = default;

		class Output {
		public:
			virtual ~Output() = default;
			virtual bool open() noexcept = 0;
			virtual bool write(const uint8_t* data, size_t size) noexcept = 0;
			virtual std::ostream& getOutputStream() noexcept = 0;
			virtual bool close() noexcept = 0;
		};
		virtual Output* getOutput()  const noexcept = 0;

		class Input {
		public:
			virtual ~Input() = default;
			virtual size_t open() noexcept = 0;
			virtual std::unique_ptr<const char[]> read() noexcept = 0;
			virtual bool close() noexcept = 0;
			virtual const char* getName() const noexcept = 0;
		};
		virtual Input* getInput() const noexcept = 0;

		virtual std::string toString() const noexcept = 0;

		bool isDebug() const noexcept {
			return mDebug;
		}


		OutputFormat getOutputFormat() const noexcept {
			return mOutputFormat;
		}

		bool isValid() const noexcept {
			return mIsValid;
		}

		Optimization getOptimizationLevel() const noexcept {
			return mOptimizationLevel;
		}

		void setOptimizationLevel(Optimization level) noexcept {
			mOptimizationLevel = level;
		}

		Metadata getReflectionTarget() const noexcept {
			return mReflectionTarget;
		}


		bool printShaders() const noexcept {
			return mPrintShaders;
		}

		bool saveRawVariants() const noexcept {
			return mSaveRawVariants;
		}

		bool rawShaderMode() const noexcept {
			return mRawShaderMode;
		}

		bool noSamplerValidation() const noexcept {
			return mNoSamplerValidation;
		}

		bool includeEssl1() const noexcept {
			return mIncludeEssl1;
		}


		const StringReplacementMap& getDefines() const noexcept {
			return mDefines;
		}

		const StringReplacementMap& getTemplateMap() const noexcept {
			return mTemplateMap;
		}

		const StringReplacementMap& getMaterialParameters() const noexcept {
			return mMaterialParameters;
		}


	protected:
		bool mDebug = false;
		bool mIsValid = true;
		bool mPrintShaders = false;
		bool mRawShaderMode = false;
		bool mNoSamplerValidation = false;
		bool mSaveRawVariants = false;
		Optimization mOptimizationLevel = Optimization::PERFORMANCE;
		Metadata mReflectionTarget = Metadata::NONE;
		OutputFormat mOutputFormat = OutputFormat::BLOB;
		StringReplacementMap mDefines;
		StringReplacementMap mTemplateMap;
		StringReplacementMap mMaterialParameters;
		
		bool mIncludeEssl1 = true;
	};

}
