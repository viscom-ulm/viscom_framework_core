/**
 * @file   GPUProgram.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Implementation of the GPU program class.
 */

#include "GPUProgram.h"
#include <iostream>
#include "core/FrameworkInternal.h"
#include "core/gfx/Shader.h"
#include "core/open_gl.h"
#include "core/utils/utils.h"

namespace viscom {

    /**
     *  Constructor.
     *  @param theProgramName the name of the program used to identify during logging.
     *  @param node the application object for dependencies.
     *  @param synchronize defines if the GPU program is a synchronized resource.
     */
    GPUProgram::GPUProgram(const std::string& theProgramName, FrameworkInternal* node, bool synchronize) :
        Resource(theProgramName, ResourceType::GPUProgram, node, synchronize),
        programName_(theProgramName),
        program_(0)
    {
    }

    void GPUProgram::Initialize(std::initializer_list<std::string> shaderNames)
    {
        shaderNames_ = std::vector<std::string>(shaderNames);
        InitializeFinished();
    }

    void GPUProgram::Initialize(std::vector<std::string> shaderNames)
    {
        shaderNames_ = shaderNames;
        InitializeFinished();
    }

    void GPUProgram::Initialize(std::vector<std::string> shaderNames, const std::vector<std::string>& defines)
    {
        shaderNames_ = shaderNames;
        defines_ = defines;
        InitializeFinished();
    }

    /** Destructor. */
    GPUProgram::~GPUProgram() noexcept
    {
        if (this->program_ != 0) {
            glDeleteProgram(this->program_);
            this->program_ = 0;
        }
    }

    // ReSharper disable CppDoxygenUnresolvedReference
    /**
     *  Links a new program.
     *  @tparam T the type of the shaders list.
     *  @param name the name of the program.
     *  @param shaders a list of shaders used for creating the program.
     *  @param shaderAccessor function to access the shader id from the shader object in list.
     */
    // ReSharper restore CppDoxygenUnresolvedReference
    template<typename T, typename SHAcc>
    GLuint GPUProgram::linkNewProgram(const std::string& name, const std::vector<T>& shaders, SHAcc shaderAccessor)
    {
        auto program = glCreateProgram();
        if (program == 0) {
            std::cerr << "Could not create GPU program!";
            throw resource_loading_error(name, "Could not create GPU program!");
        }
        for (const auto& shader : shaders) {
            glAttachShader(program, shaderAccessor(shader));
        }
        glLinkProgram(program);

        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            GLint infoLogLength;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

            std::string strInfoLog;
            strInfoLog.resize(static_cast<std::size_t>(infoLogLength) + 1);
            glGetProgramInfoLog(program, infoLogLength, nullptr, strInfoLog.data());
            std::cerr << "Linker failure: " << strInfoLog;

            for (const auto& shader : shaders) {
                glDetachShader(program, shaderAccessor(shader));
            }
            glDeleteProgram(program);

            throw shader_compiler_error(name, strInfoLog);
        }
        for (const auto& shader : shaders) {
            glDetachShader(program, shaderAccessor(shader));
        }
        return program;
    }

    /**
     *  Recompiles the program.
     */
    void GPUProgram::recompileProgram()
    {
        auto tmp = std::optional<std::vector<std::uint8_t>>();
        Load(tmp);
    }

    GLint GPUProgram::getUniformLocation(const std::string& name) const
    {
        return glGetUniformLocation(program_, name.c_str());
    }

    std::vector<GLint> GPUProgram::getUniformLocations(const std::initializer_list<std::string>& names) const
    {
        return GetUniformLocations(names);
    }

    std::vector<GLint> GPUProgram::GetUniformLocations(const std::vector<std::string>& names) const
    {
        std::vector<GLint> result;
        result.reserve(names.size());
        for (const auto& name : names) {
            result.push_back(glGetUniformLocation(program_, name.c_str()));
        }
        return result;
    }

    GLint GPUProgram::getAttributeLocation(const std::string& name) const
    {
        return glGetAttribLocation(program_, name.c_str());
    }

    std::vector<GLint> GPUProgram::getAttributeLocations(const std::initializer_list<std::string>& names) const
    {
        return GetAttributeLocations(names);
    }

    std::vector<GLint> GPUProgram::GetAttributeLocations(const std::vector<std::string>& names) const
    {
        std::vector<GLint> result;
        result.reserve(names.size());
        for (const auto& name : names) {
            result.push_back(glGetAttribLocation(program_, name.c_str()));
        }
        return result;
    }

    void GPUProgram::LoadProgram(viscom::function_view<std::unique_ptr<Shader>(const std::string&, const FrameworkInternal*)> createShader)
    {
        ShaderList oldShaders = std::move(shaders_);
        GLuint oldProgram = program_;

        try {
            for (const auto& shaderName : shaderNames_) {
                shaders_.emplace_back(createShader(shaderName, GetAppNode()));
            }
            program_ = linkNewProgram(programName_, shaders_, [](const std::unique_ptr<Shader>& shdr) noexcept { return shdr->getShaderId(); });
        }
        catch (shader_compiler_error&) {
            program_ = oldProgram;
            shaders_ = std::move(oldShaders);
            throw;
        }
    }

    void GPUProgram::Load(std::optional<std::vector<std::uint8_t>>& data)
    {
        LoadProgram([this](const std::string& shaderName, const FrameworkInternal* node) { return std::make_unique<Shader>(shaderName, node, defines_); });
        if (data.has_value()) {
            data->clear();

            std::size_t programSize = 0;
            for (const auto& shader : shaders_) {
                programSize += sizeof(std::size_t);
                programSize += shader->GetSource().size() * sizeof(std::remove_reference_t<decltype(shader->GetSource())>::value_type);
            }

            data->resize(programSize);
            auto dataptr = data->data();
            for (const auto& shader : shaders_) {
                auto shaderSize = shader->GetSource().size() * sizeof(std::remove_reference_t<decltype(shader->GetSource())>::value_type);
                *reinterpret_cast<std::size_t*>(dataptr) = shaderSize;
                dataptr += sizeof(std::size_t);
                utils::memcpyfaster(dataptr, shader->GetSource().data(), shaderSize);
                dataptr += shaderSize;
            }
        }
    }

    void GPUProgram::LoadFromMemory(const void* data, std::size_t size)
    {
        _unused(size);
        std::unordered_map<std::string, std::string> shaderNameMap;
        std::size_t totalSize = 0;
        auto dataPtr = reinterpret_cast<const std::uint8_t*>(data);
        for (const auto& shaderName : shaderNames_) {
            auto shaderPtr = reinterpret_cast<const std::size_t*>(dataPtr);
            auto shaderSize = sizeof(std::size_t) + shaderPtr[0];
            totalSize += shaderSize;
            assert(totalSize <= size);
            std::string shader;
            shader.resize(shaderPtr[0]);
            utils::memcpyfaster(shader.data(), &shaderPtr[1], shaderPtr[0]);

            dataPtr += shaderSize;
            shaderNameMap[shaderName] = shader;
        }

        LoadProgram([&shaderNameMap](const std::string& shaderName, const FrameworkInternal* node) { return std::make_unique<Shader>(shaderName, node, shaderNameMap[shaderName]); });
    }
}
