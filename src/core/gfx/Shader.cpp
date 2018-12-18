/**
 * @file   Shader.cpp
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Implementation of the shader helper class.
 */

#include "Shader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <iterator>
#ifndef __APPLE_CC__
#include <experimental/filesystem>
#endif
#include "core/FrameworkInternal.h"
#include <regex>
#include "core/open_gl.h"

namespace viscom {

    /** Header inclusion depth limit. */
    constexpr unsigned int MAX_INCLUDE_RECURSION_DEPTH = 32; //-V112

    /**
     * Constructor.
     * @param shader the name of the shader the error occurred in
     * @param errors the error string OpenGL returned
     */
    shader_compiler_error::shader_compiler_error(const std::string& shader, const std::string& errors) noexcept :
        resource_loading_error(shader, errors),
        shr_(shader.size() + 1),
        errs_(errors.size() + 1),
        myWhat_(0)
    {
        auto result = "Shader " + shader + " compilation failed!\n";
        result += "Compiler errors:\n" + errors;
        myWhat_.resize(result.size() + 1);

        std::copy(shader.begin(), shader.end(), shr_.begin());
        shr_[shader.size()] = '\0';
        std::copy(errors.begin(), errors.end(), errs_.begin());
        errs_[errors.size()] = '\0';
        std::copy(result.begin(), result.end(), myWhat_.begin());
        myWhat_[result.size()] = '\0';
    }

    /**
     *  Copy-constructor.
     *  @param rhs the object to copy from.
     */
    shader_compiler_error::shader_compiler_error(const shader_compiler_error& rhs) noexcept :
    shader_compiler_error(std::string(rhs.shr_.data()), std::string(rhs.errs_.data()))
    {
    }

    /**
     *  Copy-assignment operator.
     *  @param rhs the object to copy from.
     */
    shader_compiler_error& shader_compiler_error::operator=(const shader_compiler_error& rhs) noexcept
    {
        shader_compiler_error tmp(std::string(rhs.shr_.data()), std::string(rhs.errs_.data()));
        *this = std::move(tmp);
        return *this;
    }

    /**
     *  Move-constructor.
     *  @param rhs the object to move.
     */
    shader_compiler_error::shader_compiler_error(shader_compiler_error&& rhs) noexcept :
        resource_loading_error(std::move(rhs)),
        shr_(std::move(rhs.shr_)),
        errs_(std::move(rhs.errs_)),
        myWhat_(std::move(rhs.myWhat_))
    {
    }

    /**
     *  Move-assignment operator.
     *  @param rhs the object to move.
     */
    shader_compiler_error& shader_compiler_error::operator= (shader_compiler_error&& rhs) noexcept
    {
        resource_loading_error* tExcpt = this;
        *tExcpt = std::move(static_cast<resource_loading_error&&>(rhs));
        if (this != &rhs) {
            shr_ = std::move(rhs.shr_);
            errs_ = std::move(rhs.errs_);
            myWhat_ = std::move(rhs.myWhat_);
        }
        return *this;
    }

    /** Returns information about the exception */
    const char* shader_compiler_error::what() const noexcept
    {
        return myWhat_.data();
    }

    /**
     *  Constructor.
     *  @param shaderFilename the shader file name
     *  @param node the application holding the configuration to retrieve the search paths.
     */
    Shader::Shader(const std::string& shaderFilename, const FrameworkInternal* node) :
        Shader{ shaderFilename, node, std::vector<std::string>{} }
    {
    }

    /**
     *  Constructor.
     *  @param shaderFilename the shader file name
     *  @param node the application holding the configuration to retrieve the search paths.
     *  @param defines the defines to be used in the shader.
     */
    Shader::Shader(const std::string& shaderFilename, const FrameworkInternal* node, const std::vector<std::string>& defines) :
        Shader{ shaderFilename, node, LoadShaderFile(shaderFilename, defines, node) }
    {
        defines_ = defines;
    }

    /**
     *  Constructor.
     *  @param shaderFilename the shader file name
     *  @param node the application holding the configuration to retrieve the search paths.
     *  @param shader source code of the shader.
     */
    Shader::Shader(const std::string& shaderFilename, const FrameworkInternal* node, const std::string& shader) :
        filename_{ shaderFilename },
        shader_{ 0 },
        type_{ GL_VERTEX_SHADER },
        strType_{ "vertex" },
        generatedSource_{ shader }
    {
        if (utils::endsWith(shaderFilename, ".frag")) {
            type_ = GL_FRAGMENT_SHADER;
            strType_ = "fragment";
        }
        else if (utils::endsWith(shaderFilename, ".geom")) {
            type_ = GL_GEOMETRY_SHADER;
            strType_ = "geometry";
        }
        else if (utils::endsWith(shaderFilename, ".tesc")) {
            type_ = GL_TESS_CONTROL_SHADER;
            strType_ = "tessellation control";
        }
        else if (utils::endsWith(shaderFilename, ".tese")) {
            type_ = GL_TESS_EVALUATION_SHADER;
            strType_ = "tessellation evaluation";
        }
        else if (utils::endsWith(shaderFilename, ".comp")) {
            type_ = GL_COMPUTE_SHADER;
            strType_ = "compute";
        }
        shader_ = CompileShader(filename_, generatedSource_, type_, strType_);
    }

    /**
     *  Move-constructor.
     *  @param rhs the object to move.
     */
    Shader::Shader(Shader&& rhs) noexcept :
        filename_{ std::move(rhs.filename_) },
        shader_{ std::move(rhs.shader_) },
        type_{ std::move(rhs.type_) },
        strType_{ std::move(rhs.strType_) },
        defines_{ std::move(rhs.defines_) }
    {
        rhs.shader_ = 0;
    }

    /**
     * Move-assignment operator.
     * @param rhs the object to move.
     * @return reference to this object.
     */
    Shader& Shader::operator =(Shader&& rhs) noexcept
    {
        if (this != &rhs) {
            this->~Shader();
            filename_ = std::move(rhs.filename_);
            shader_ = rhs.shader_;
            rhs.shader_ = 0;
            type_ = std::move(rhs.type_);
            strType_ = std::move(rhs.strType_);
            defines_ = std::move(rhs.defines_);
        }
        return *this;
    }

    /** Destructor. */
    Shader::~Shader() noexcept
    {
        if (this->shader_ != 0) {
            glDeleteShader(shader_);
            shader_ = 0;
        }
    }

    /**
     *  Loads a shader from file and recursively adds all includes.
     *  taken from https://github.com/dasmysh/OGLFramework_uulm/blob/c4548e84d29bc16b53360f65227597530306c686/OGLFramework_uulm/gfx/glrenderer/Shader.cpp
     *  licensed under MIT by Sebastian Maisch
     *  @param filename the name of the file to load.
     *  @param defines the defines to add at the beginning.
     *  @param node the application holding the configuration to retrieve the search paths.
     */
    std::string Shader::LoadShaderFile(const std::string& filename, const std::vector<std::string>& defines, const FrameworkInternal* node)
    {
        auto fullFilename = Resource::FindResourceLocation("shader/" + filename, node);
        unsigned int fileId{ 0 };
        auto shaderText = LoadShaderFileRecursive(fullFilename, defines, fileId, 0);
        std::ofstream shader_out(fullFilename + ".gen");
        shader_out << shaderText;
        shader_out.close();

        return shaderText;
    }

    /**
     *  Loads a shader from file and recursively adds all includes.
     *  @see LoadShaderFile.
     */
    std::string Shader::LoadShaderFileRecursive(const std::string & filename, const std::vector<std::string>& defines, unsigned int& fileId, unsigned int recursionDepth)
    {
#ifdef __APPLE_CC__
        if (!defines.empty()) LOG(WARNING) << "Defines and includes in shaders not supported on MacOS.";
        std::ifstream file(filename.c_str(), std::ifstream::in);
        if (!file) {
            LOG(WARNING) << "Could not load shader file!";
            std::cerr << "Could not load shader file!";
            throw resource_loading_error(filename, "Could not load shader file!");
        }
        std::string line;
        std::stringstream content;
        while (file.good()) {
            std::getline(file, line);
            content << line << std::endl;
    }
        file.close();
        return content.str();
#else
        if (recursionDepth > MAX_INCLUDE_RECURSION_DEPTH) {
            LOG(WARNING) << L"Header inclusion depth limit reached! Cyclic header inclusion?";
            throw resource_loading_error(filename, "Header inclusion depth limit reached! Cyclic header inclusion? File " + filename);
        }
        namespace filesystem = std::experimental::filesystem;
        filesystem::path sdrFile{ filename };
        auto currentPath = sdrFile.parent_path().string() + "/";
        std::ifstream file(filename.c_str(), std::ifstream::in);
        std::string line;
        std::stringstream content;
        unsigned int lineCount = 1;
        auto nextFileId = fileId + 1;

        while (file.good()) {
            std::getline(file, line);
            auto trimedLine = utils::trim(line);

            static const std::regex re(R"(^[ ]*#[ ]*include[ ]+["<](.*)[">].*)");
            std::smatch matches;
            if (std::regex_search(line, matches, re)) {
                auto includeFile = currentPath + matches[1].str();
                if (!filesystem::exists(includeFile)) {
                    LOG(WARNING) << filename.c_str() << L"(" << lineCount << R"() : fatal error: cannot open include file ")"
                        << includeFile.c_str() << R"(".)";
                    throw resource_loading_error(filename, "Cannot open include file: " + includeFile);
                }
                content << "#line " << 1 << " " << nextFileId << std::endl;
                content << LoadShaderFileRecursive(includeFile, std::vector<std::string>(), nextFileId, recursionDepth + 1);
                content << "#line " << lineCount + 1 << " " << fileId << std::endl;
            }
            else {
                content << line << std::endl;
            }

            if (utils::beginsWith(trimedLine, "#version")) {
                for (const auto& def : defines) {
                    auto trimedDefine = utils::trim(def);
                    content << "#define " << trimedDefine << std::endl;
                }
                content << "#line " << lineCount + 1 << " " << fileId << std::endl;
            }
            ++lineCount;
        }

        file.close();
        fileId = nextFileId;
        return content.str();
#endif
    }

    /**
     *  Loads a shader from file and compiles it.
     *  @param filename the shader file name.
     *  @param shaderText the shader code as a string.
     *  @param type the shader type.
     *  @param strType the shader type as string.
     *  @return the compiled shader if successful.
     */
    GLuint Shader::CompileShader(const std::string& filename, const std::string& shaderText, GLenum type, const std::string& strType)
    {
        auto shader = glCreateShader(type);
        if (shader == 0) {
            LOG(WARNING) << "Could not create shader!";
            std::cerr << "Could not create shader!";
            throw resource_loading_error(filename, "Could not create shader!");
        }
        auto shaderTextArray = shaderText.c_str();
        auto shaderLength = static_cast<int>(shaderText.length());
        glShaderSource(shader, 1, &shaderTextArray, &shaderLength);
        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            GLint infoLogLength;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

            std::string strInfoLog;
            strInfoLog.resize(static_cast<std::size_t>(infoLogLength + 1));
            glGetShaderInfoLog(shader, infoLogLength, nullptr, strInfoLog.data());

            LOG(WARNING) << "Compile error in " << strType << " shader (" << filename.c_str() << "): "
                << std::endl << strInfoLog;
            std::cerr << "Compile error in " << strType << " shader (" << filename.c_str() << "): "
                << std::endl << strInfoLog;
            glDeleteShader(shader);
            throw shader_compiler_error(filename, strInfoLog);
        }
        return shader;
    }
}
