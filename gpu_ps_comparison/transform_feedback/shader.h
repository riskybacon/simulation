#ifndef _shader_h
#define _shader_h

#include <string>
#include <sstream>
#include <map>
#include <stdexcept>
#include <vector>
#include "opengl.h"
#ifdef USE_GUTZ
#include <mat4.h>
#include <mat2.h>
#include <vec.h>
#endif

#include <glm/glm.hpp>

namespace GL
{

   // Macros for making sure that uniforms and attributes exists.
   // Can be turned off by not defining _DEBUG
#ifdef _DEBUG
#define ASSERT_UNIFORM_EXISTS(_name) \
{ \
   std::map<std::string, GLuint>::const_iterator loc = _uniform.find(name); \
   if(loc == _uniform.end()) \
   { \
      std::ostringstream uniform_assert__out; \
      uniform_assert__out << "Uniform does not exist: " << name << std::endl; \
      throw std::runtime_error(uniform_assert__out.str());                    \
   }\
}                                                                  

#define ASSERT_ATTRIBUTE_EXISTS(_name) \
{ \
   std::map<std::string, GLuint>::const_iterator loc = _attrib.find(name); \
   if(loc == _attrib.end()) \
   { \
      std::ostringstream attribute_assert__out; \
      attribute_assert__out << "Error in file " << __FILE__ << ":" << __LINE__ << "\n";  \
      attribute_assert__out << GL_FUNCTION_NAME << ".\n\n";                                     \
      attribute_assert__out << "Attribute does not exist: " << name << std::endl; \
      throw std::runtime_error(attribute_assert__out.str());                    \
   }\
}                                                                  

#define ASSERT_VARYING_EXISTS(_name) \
{ \
   std::map<std::string, GLuint>::const_iterator loc = _varying.find(name); \
   if(loc == _varying.end()) \
   { \
      std::ostringstream varying_assert__out; \
      varying_assert__out << "Varying variable does not exist: " << name << std::endl; \
      throw std::runtime_error(varying_assert__out.str());                    \
   }\
}

#else
#define ASSERT_UNIFORM_EXISTS(_name)
#define ASSERT_ATTRIBUTE_EXISTS(_name)
#define ASSERT_VARYING_EXISTS(_name)
#endif
   
   /**
    * Turn OpenGL errors into strings
    */
   std::string errorString(GLenum error);

   /**
    * An OpenGL GLSL shader
    */
   class Shader
   {
   public:
      /**
       * Create a shader program from a file. The caller should check the
       * compile status
       *
       * @param filename      The name of the file with the shader source
       * @param shaderType    The type of shader (GL_VERTEX_SHADER, etc)
       */
      Shader(const std::string& filename, GLenum shaderType);
      
      /**
       * Destructor
       */
      ~Shader();
      
      /**
       * Check the compile status of a shader
       *
       * @param shader     Handle to a shader
       * @return           true if the shader was compiled, false otherwise
       */
      bool getCompileStatus() const;
      
      /**
       * Retrieve a shader log
       *
       * @return           The contents of the log
       */
      std::string getLog(void) const;
      
      /**
       * @return OpenGL handle for the shader
       */
      GLuint getHandle(void) const
      {
         return _handle;
      }

   private:
      GLuint _handle; //< OpenGL handle for a GLSL shader
   };
   
   /**
    * An OpenGL GLSL program
    */
   class Program
   {
   public:

      /**
       * Create a GLSL program
       *
       * @param vertexFile
       *    The name of the file that contains vertex shader source
       * @param varyings
       *    names of the transform feedback varyings
       * @param buffermode
       *    Buffer mode for the transform feedback: GL_INTERLEAVED_ATTRIBUTES or GL_SEPARATE_ATTRIBS
       */
      Program(const std::string& vertexFile, const std::vector<std::string>& varyings, GLenum buffermode);

      /**
       * Create a GLSL program
       *
       * @param vertexFile
       *    The name of the file that contains vertex shader source
       * @param fragmentFile
       *    The name of the file that contains the fragment shader source
       */
      Program(const std::string& vertexFile, const std::string& fragmentFile);

      /**
       * Create a GLSL program
       *
       * @param vertexFile
       *    The name of the file that contains vertex shader source
       * @param fragmentFile
       *    The name of the file that contains the fragment shader source
       * @param geometryFile
       *    The name of the file that contains the geometry shader source
       */
      Program(const std::string& vertexFile, const std::string& fragmentFile,
              const std::string& geometryFile);

      /**
       * Destructor
       */
      ~Program();

      /**
       * Map the names of uniforms to indices
       */
      void mapUniformNamesToIndices(void);
      
      /**
       * Map the names of attributes to indices
       */
      void mapAttributeNamesToIndices(void);

      /**
       * Build a mapping of varying names to indices
       */
      void mapVaryingNamesToIndices();

      /**
       * Check the link status of the program
       *
       * @param shader     Handle to a shader
       * @return           true if the shader was compiled, false otherwise
       */
      bool getLinkStatus() const;
      
      /**
       * Retrieve the program log
       *
       * @return           The contents of the log
       */
      std::string getLog(void) const;
      
      /**
       * Get the location of an program attribute
       *
       * @param name
       *    The name of the attribute
       * @return the location of the program attribute
       */
      GLuint getAttribLocation(const std::string& name) const
      {
         ASSERT_ATTRIBUTE_EXISTS(name);
         return glGetAttribLocation(_handle, name.c_str());
      }

      /**
       * Get the location of an varying variable
       *
       * @param name
       *    The name of the varying variable
       * @return the location of the varying variable
       */
      GLuint getVaryingLocation(const std::string& name) const
      {
         ASSERT_VARYING_EXISTS(name);
         std::map<std::string, GLuint>::const_iterator loc = _varying.find(name);
         return loc->second;
      }

      /**
       * Get the location of an program uniform variable
       *
       * @param name
       *    The name of the uniform variable
       * @return the location of the uniform variable
       */
      GLuint getUniformLocation(const std::string& name) const
      {
         return glGetUniformLocation(_handle, name.c_str());
      }
      
      /**
       * @return OpenGL handle for the shader
       */
      GLuint getHandle(void) const
      {
         return _handle;
      }
      
      /**
       * Bind this program to the current OpenGL state
       */
      void bind(void)
      {
         glUseProgram(_handle);
      }
      
      //#ifdef OPENGL3
      /**
       * Change back to fixed function pipeline. Not available for OpenGL 3 programs
       */
      void release(void)
      {
         glUseProgram(0);
      }
      //#endif
      
      /**
       * @return the number of shader objects attached to program.
       */
      const GLint getAttachedShaders(void) const
      {
         GLint params;
         glGetProgramiv(_handle, GL_ATTACHED_SHADERS, &params);
         return params;
      }
      
      /**
       * @return the number of active attribute variables for program.
       */
      const GLint getActiveAttributes(void) const
      {
         GLint params;
         glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTES, &params);
         return params;
      }
      
      /**
       * @return the length of the longest active attribute name for program, including
       *   the null termination character (i.e., the size of the character buffer required
       *   to store the longest attribute name). If no active attributes exist, 0 is 
       *   returned.
       */
      const GLint getActiveAttributeMaxLength(void) const
      {
         GLint params;
         glGetProgramiv(_handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &params);
         return params;
      }
      
      /**
       * @return the number of active uniform variables for program.
       */
      const GLint getActiveUniforms(void) const
      {
         GLint params;
         glGetProgramiv(_handle, GL_ACTIVE_UNIFORMS, &params);
         return params;
      }

      /**
       * Get the name of a uniform variable at the specified index
       *
       * @param index
       *   The index for the uniform
       *
       * @return The name of the uniform
       */
      const std::string getUniformName(GLuint index) const
      {
         static const GLsizei maxNameSize = 256;
         GLchar  glName[maxNameSize];
         GLsizei length;
         GLint   size;
         GLenum  type;
         glGetActiveUniform(_handle, index, maxNameSize, &length, &size, &type, glName);
         std::string name(glName);
         return name;
      }

      /**
       * @return the length of the longest active uniform variable name for program,
       *   including the null termination character (i.e., the size of the character buffer
       *   required to store the longest uniform variable name). If no active uniform 
       *   variables exist, 0 is returned.
       */
      const GLint getActiveUniformMaxLength(void) const
      {
         GLint params;
         glGetProgramiv(_handle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &params);
         return params;
      }

      //{@ glUniform1i
      void setUniform(const std::string& name, const GLint v0) {
         glUniform1i(_uniform[name], v0);
      }
      

      void setUniform(const GLint id, const GLint v0) {
         glUniform1i(id, v0);
      }
      
      void setUniform1i(const std::string& name, const GLint v0)
      {
         glUniform1i(_uniform[name], v0);
      }
      
      void setUniform1i(const GLint id, const GLint v0) {
         glUniform1i(id, v0);
      }

      
      void setUniform(const std::string& name, const size_t v0) {
         glUniform1i(_uniform[name], v0);
      }
      
      void setUniform(const GLint id, const size_t v0) {
         glUniform1i(id, v0);
      }
      

      
      //@}
      
      //{@ glUniform1f
      void setUniform(const std::string& name, const GLfloat v0)
      {
         glUniform1f(_uniform[name], v0);
      }

      void setUniform(const GLint id, GLfloat v0)
      {
         glUniform1f(id, v0);
      }
      
      void setUniform1f(const std::string& name, const GLfloat v0)
      {
         glUniform1f(_uniform[name], v0);
      }
      
      void setUniform1f(const GLint id, GLfloat v0)
      {
         glUniform1f(id, v0);
      }
      //@}
      
#if 0
      void setUniform(const std::string& name, GLfloat v0, GLfloat v1)
      {
         glUniform2f(_uniform[name], v0, v1);
         
      }

      void setUniform(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2)
      {
         glUniform3f(_uniform[name], v0, v1, v2);
         
      }

      void setUniform(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)
      {
         glUniform4f(_uniform[name], v0, v1, v2, v3);
      }
      

      
      void setUniform(const std::string& name, GLint v0, GLint v1)
      {
         glUniform2i(_uniform[name], v0, v1);
      }
      
      void setUniform(const std::string& name, GLint v0, GLint v1, GLint v2)
      {
         glUniform3i(_uniform[name], v0, v1, v2);
      }
      
      void setUniform(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3)
      {
         glUniform4i(_uniform[name], v0, v1, v2, v3);
      }
      
      void setUniform1ui(const std::string& name, GLuint v0)
      {
         glUniform1ui(_uniform[name], v0);
         
      }
      
      void setUniform(const std::string& name, GLuint v0, GLuint v1)
      {
         glUniform2ui(_uniform[name], v0, v1);
         
      }
      
      void setUniform(const std::string& name, GLuint v0, GLuint v1, GLuint v2)
      {
         glUniform3ui(_uniform[name], v0, v1, v2);
         
      }
      
      void setUniform(const std::string& name, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
      {
         glUniform4ui(_uniform[name], v0, v1, v2, v3);
      }

      void setUniform1uiv(const std::string& name, GLsizei length, GLuint* data) 
      {
         glUniform1uiv(_uniform[name], length, data);
      }
      
      void setUniform1uiv(const GLint id, GLsizei length, GLuint* data) 
      {
         glUniform1uiv(id, length, data);
      }
#endif

      void setUniform(const std::string& name, const std::vector<int>& data) {
         glUniform1iv(_uniform[name], data.size(), &data[0]);
      }
      
      /**
       * Modifies the value of a uniform variable array
       *
       * @param name
       * Specifies the name of the uniform var
       *
       * @param count
       * Specifies the number of matrices that are to be modified. This
       * should be 1 if the targeted uniform variable is not an array of
       * matrices, and 1 or more if it is an array of matrices.
       *
       * @param transpose
       * Specifies whether to transpose the matrix as the values are loaded
       * into the uniform variable.
       *
       * @param value
       * Specifies a pointer to an array of count values that will be used
       * to update the specified uniform variable.
       */
      void setUniformMatrix4(const std::string& name, GLsizei count, GLboolean transpose,
                               const GLfloat* value)
      {
#ifdef ROBUST_UNIFORM_LOCATIONS
         glUniformMatrix4fv(getUniformLocation(name), count, transpose, value);
#else
         glUniformMatrix4fv(_uniform[name], count, transpose, value);
#endif
      }
      
      void setUniform4(const std::string& name, GLsizei count, const GLfloat *value)
      {
#ifdef ROBUST_UNIFORM_LOCATIONS
         glUniform4fv(getUniformLocation(name), count, value);
#else
         glUniform4fv(_uniform[name], count, value);
#endif
      }

#ifdef USE_GUTZ
      void setUniform(const std::string& name, const gutz::mat4f& mat, GLboolean transpose = GL_FALSE)
      {
         ASSERT_UNIFORM_EXISTS(name);
         glUniformMatrix4fv(_uniform[name], 1, transpose, mat.m);
      }

      void setUniform(const std::string& name, const gutz::mat3f& mat, GLboolean transpose = GL_FALSE)
      {
         ASSERT_UNIFORM_EXISTS(name);
         glUniformMatrix3fv(_uniform[name], 1, transpose, mat.m);
      }

      template<typename T>
      void setUniform(const std::string& name, const gutz::mat2<T>& mat, GLboolean transpose = GL_FALSE)
      {
         ASSERT_UNIFORM_EXISTS(name);
         glUniformMatrix2fv(_uniform[name], 1, transpose, mat.m);
      }
      
      void setUniform(const std::string& name, const gutz::vec<float,4>& v)
      {
         ASSERT_UNIFORM_EXISTS(name);
         glUniform4fv(_uniform[name], 1, v.v);
      }
#endif
      
      void setUniform(const std::string& name, const glm::mat4& mat, GLboolean transpose = GL_FALSE)
      {
         ASSERT_UNIFORM_EXISTS(name);
         glUniformMatrix4fv(_uniform[name], 1, transpose, &mat[0][0]);
      }

      void setUniform(const std::string& name, const glm::mat3& mat, GLboolean transpose = GL_FALSE)
      {
         ASSERT_UNIFORM_EXISTS(name);
         glUniformMatrix3fv(_uniform[name], 1, transpose, &mat[0][0]);
      }

      void setUniform(const std::string& name, const std::vector<glm::mat3>& mat, GLboolean transpose = GL_FALSE)
      {
         ASSERT_UNIFORM_EXISTS(name);
         glUniformMatrix3fv(_uniform[name], mat.size(), transpose    , &mat[0][0][0]);
      }

      void setUniform(const std::string& name, const std::vector<glm::mat4>& mat, GLboolean transpose = GL_FALSE)
      {
         ASSERT_UNIFORM_EXISTS(name);
         glUniformMatrix4fv(_uniform[name], mat.size(), transpose, &mat[0][0][0]);
      }

      void setUniform(const std::string& name, const glm::vec4& v)
      {
         ASSERT_UNIFORM_EXISTS(name);
         glUniform4fv(_uniform[name], 1, &v[0]);
      }


      void setUniform(const std::string& name, const glm::vec3& v)
      {
         ASSERT_UNIFORM_EXISTS(name);
         glUniform3fv(_uniform[name], 1, &v[0]);
      }

      void setUniform(const std::string& name, const glm::vec2& v)
      {
         ASSERT_UNIFORM_EXISTS(name);
         glUniform2fv(_uniform[name], 1, &v[0]);
      }

      
      void setUniform(const std::string& name, std::vector<glm::vec2>& v)
      {
         ASSERT_UNIFORM_EXISTS(name);
         GLfloat* ptr = &v[0][0];
         glUniform2fv(_uniform[name], v.size(), ptr);
         GL_ERR_CHECK();
      }

   private:
      
      GLuint                        _handle;         //< OpenGL handle for a GLSL shader
      Shader*                       _vertexShader;   //< Pointer to the fragment shader
      Shader*                       _fragmentShader; //< Pointer to the vertex shader
      Shader*                       _geometryShader; //< Pointer to the geometry shader
      std::map<std::string, GLuint> _uniform;        //< Map of uniform names to GLuint indices
      std::map<std::string, GLuint> _attrib;         //< Map of attribute names to GLuint indices
      std::map<std::string, GLuint> _varying;        //< Map of varying names to GLuint indices
   };
}
#endif
