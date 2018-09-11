/**
 * @file   Shader.h
 * @author Sebastian Maisch <sebastian.maisch@uni-ulm.de>
 * @date   2016.12.14
 *
 * @brief  Declaration of the shader helper class.
 */

#pragma once

#include "core/main.h"
#include "core/open_gl_fwd.h"
#include "core/resources/ResourceManager.h"

namespace viscom {

    class FrameworkInternal;

    /**
     * Exception class for shader compiler errors.
     */
    class shader_compiler_error : public resource_loading_error
    {
    public:
        shader_compiler_error(const std::string& shader, const std::string& errors) noexcept;
        shader_compiler_error(const shader_compiler_error&) noexcept;
        shader_compiler_error& operator=(const shader_compiler_error&) noexcept;
        shader_compiler_error(shader_compiler_error&&) noexcept;
        shader_compiler_error& operator=(shader_compiler_error&&) noexcept;
        const char* what() const noexcept;

    private:
        /** Holds the shader name. */
        std::vector<char> shr_;
        /** Holds the errors. */
        std::vector<char> errs_;
        /** holds the error message. */
        std::vector<char> myWhat_;
    };

    /**
     * Wrapper class for shader loading.
     */
    class Shader final
    {
    public:
        Shader(const std::string& shaderFilename, const FrameworkInternal* node);
        Shader(const std::string& shaderFilename, const FrameworkInternal* node, const std::vector<std::string>& defines);
        Shader(const std::string& shaderFilename, const FrameworkInternal* node, const std::string& shader);
        Shader(const Shader& orig) = delete;
        Shader& operator=(const Shader&) = delete;
        Shader(Shader&& orig) noexcept;
        Shader& operator=(Shader&& orig) noexcept;
        ~Shader() noexcept;

        /** Returns the OpenGL id of the shader. */
        GLuint getShaderId() const noexcept { return shader_; }
        /** Returns the shaders source code (defines and includes added). */
        const std::string& GetSource() const { return generatedSource_; }

    private:
        /** Holds the shader file name. */
        std::string filename_;
        /** Holds the compiled shader. */
        GLuint shader_;
        /** Holds the shader type. */
        GLenum type_;
        /** Holds the shader type as a string. */
        std::string strType_;
        /** Holds the defines used in the shader. */
        std::vector<std::string> defines_;
        /** Holds the shaders source code. */
        std::string generatedSource_;

        static std::string LoadShaderFile(const std::string &filename, const std::vector<std::string> &defines, const FrameworkInternal* node);
        static std::string LoadShaderFileRecursive(const std::string &filename, const std::vector<std::string> &defines, unsigned int &fileId, unsigned int recursionDepth);
        static GLuint CompileShader(const std::string& filename, const std::string& shader, GLenum type, const std::string& strType);
    };
}
