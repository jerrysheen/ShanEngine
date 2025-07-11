#include "hzpch.h"
#include "OpenGLShader.h"

#include <fstream>
#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

namespace Hazel {

	////-------- OpenGLShaderReflection 实现 --------
	//OpenGLShaderReflection::OpenGLShaderReflection(uint32_t program)
	//	: m_RendererID(program)
	//{
	//	// TODO: 初始化反射数据
	//}

	//BufferLayout OpenGLShaderReflection::ReflectVertexInputLayout()
	//{
	//	// TODO: 解析顶点输入布局
	//	return m_InputLayout;
	//}

	//std::vector<ShaderRegisterBlock> OpenGLShaderReflection::ReflectRegisterBlocks()
	//{
	//	return std::vector<ShaderRegisterBlock>();
	//}


	//std::vector<ResourceBinding> OpenGLShaderReflection::ReflectResourceBindings()
	//{
	//	// TODO: 解析资源绑定
	//	return m_ResourceBindings;
	//}

	//Ref<ShaderRegisterBlock> OpenGLShaderReflection::GetRegisterBlockByName(const std::string& name)
	//{
	//	return Ref<ShaderRegisterBlock>();
	//}

	//Ref<ShaderRegisterBlock> OpenGLShaderReflection::GetRegisterBlockByBindPoint(uint32_t bindPoint, uint32_t space)
	//{
	//	return Ref<ShaderRegisterBlock>();
	//}

	//Ref<ShaderParameter> OpenGLShaderReflection::GetParameterByName(const std::string& name)
	//{
	//	return Ref<ShaderParameter>();
	//}


	////-------- OpenGLShader 实现 --------
	//static GLenum ShaderTypeFromString(const std::string& type) 
	//{
	//	//HZ_CORE_INFO(type);
	//	if (type == "vertex")
	//		return GL_VERTEX_SHADER;
	//	if (type == "fragment" || type == "pixel")
	//		return GL_FRAGMENT_SHADER;

	//	HZ_CORE_ASSERT(false, "Unknown shader type!");
	//	return 0;
	//}

	//OpenGLShader::OpenGLShader(const std::string& filepath)
	//{
	//	std::string shaderSource = ReadFile(filepath);
	//	auto shaderSources = PreProcess(shaderSource);
	//	Compile(shaderSources);
	//	
	//	// Extract name of file
	//	// assest/shaders/Texture.glsl
	//	auto lastSlash = filepath.find_last_of("/\\");
	//	lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
	//	auto lastDot = filepath.rfind('.');
	//	auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
	//	m_Name = filepath.substr(lastSlash, count);
	//	
	//	// 创建反射
	//	CreateReflection();
	//}
	//OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
	//	:m_Name(name)
	//{
	//	std::unordered_map<GLenum, std::string> sources;
	//	sources[GL_VERTEX_SHADER] = vertexSrc;
	//	sources[GL_FRAGMENT_SHADER] = fragmentSrc;
	//	Compile(sources);
	//	
	//	// 创建反射
	//	CreateReflection();
	//}

	//OpenGLShader::~OpenGLShader()
	//{
	//	glDeleteProgram(m_RendererID);
	//}

	//void OpenGLShader::CreateReflection()
	//{
	//	// TODO: 创建shader反射对象
	//	m_Reflection = CreateRef<OpenGLShaderReflection>(m_RendererID);
	//	
	//	// TODO: 设置输入布局
	//	m_InputLayout = m_Reflection->ReflectVertexInputLayout();
	//}

	//std::string OpenGLShader::ReadFile(const std::string& filepath) 
	//{
	//	std::string result;
	//	std::ifstream in(filepath, std::ios::in | std::ios::binary);
	//	if (in)
	//	{
	//		in.seekg(0, std::ios::end);
	//		result.resize(in.tellg());
	//		in.seekg(0, std::ios::beg);
	//		in.read(&result[0], result.size());
	//		in.close();
	//	}
	//	else
	//	{
	//		HZ_CORE_ERROR("Could not open file {0}", filepath);
	//	}
	//	return result;
	//}

	//std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& source) 
	//{
	//	std::unordered_map<GLenum, std::string> shaderSources;

	//	const char* typeToken = "#type";
	//	size_t typeTokenLength = strlen(typeToken);
	//	size_t pos = source.find(typeToken, 0);
	//	while (pos != std::string::npos)
	//	{
	//		size_t eol = source.find_first_of("\r\n", pos);
	//		HZ_CORE_ASSERT(eol != std::string::npos, "Syntax error");
	//		size_t begin = pos + typeTokenLength + 1;
	//		std::string type = source.substr(begin, eol - begin);
	//		HZ_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

	//		size_t nextLinePos = source.find_first_not_of("\r\n", eol);
	//		pos = source.find(typeToken, nextLinePos);
	//		shaderSources[ShaderTypeFromString(type)] = source.substr(nextLinePos, pos - (nextLinePos == std::string::npos ? source.size() - 1 : nextLinePos));
	//	}

	//	return shaderSources;
	//}

	//void OpenGLShader::Compile(std::unordered_map<GLenum, std::string>& shaderSources)
	//{
	//	GLuint program = glCreateProgram();
	//	std::vector<GLenum> glShaderIDs;
	//	glShaderIDs.reserve(shaderSources.size());
	//	for (const auto& kv : shaderSources) 
	//	{

	//		GLenum type = kv.first;
	//		const std::string& source = kv.second;

	//		GLuint shader = glCreateShader(type);

	//		const GLchar* sourceCStr = source.c_str();
	//		glShaderSource(shader, 1, &sourceCStr, 0);

	//		glCompileShader(shader);

	//		GLint isCompiled = 0;
	//		glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	//		if (isCompiled == GL_FALSE)
	//		{
	//			GLint maxLength = 0;
	//			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

	//			std::vector<GLchar> infoLog(maxLength);
	//			glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

	//			glDeleteShader(shader);	

	//			HZ_CORE_ERROR("{0}", infoLog.data());
	//			HZ_CORE_ASSERT(false, "Shader compilation failure!");
	//			break;
	//		}
	//		
	//		// TODO: 收集着色器字节码
	//		// m_ByteCode = ...
	//		
	//		glAttachShader(program, shader);
	//		glShaderIDs.push_back(shader);
	//	}

	//	m_RendererID = program;
	//	glLinkProgram(program);


	//	// Note the different functions here: glGetProgram* instead of glGetShader*.
	//	GLint isLinked = 0;
	//	glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
	//	if (isLinked == GL_FALSE)
	//	{
	//		GLint maxLength = 0;
	//		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

	//		// The maxLength includes the NULL character
	//		std::vector<GLchar> infoLog(maxLength);
	//		glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

	//		// We don't need the program anymore.
	//		glDeleteProgram(program);

	//		for (auto id : glShaderIDs)
	//			glDeleteShader(id);

	//		HZ_CORE_ERROR("{0}", infoLog.data());
	//		HZ_CORE_ASSERT(false, "Shader link failure!");
	//		return;
	//	}

	//	for (auto id : glShaderIDs)
	//		glDetachShader(program, id);
	//}

	//void OpenGLShader::Bind() const
	//{
	//	glUseProgram(m_RendererID);
	//}

	//void OpenGLShader::UnBind() const
	//{
	//	glUseProgram(0);
	//}


	//void OpenGLShader::SetInt(const std::string& name, int value)
	//{
	//	UploadUniformInt(name, value);
	//}

	//void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	//{
	//	UploadUniformFloat3(name, value);
	//}

	//void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	//{
	//	UploadUniformFloat4(name, value);
	//}

	//void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& matrix)
	//{
	//	UploadUniformMat4(name, matrix);
	//}

	//void OpenGLShader::SetMat3(const std::string& name, const glm::mat3& matrix)
	//{
	//	UploadUniformMat3(name, matrix);
	//}

	//void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	//{
	//	GLint location = glGetUniformLocation(m_RendererID, name.c_str());
	//	glUniform1i(location, value);
	//}

	//void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	//{
	//	GLint location = glGetUniformLocation(m_RendererID, name.c_str());
	//	glUniform1f(location, value);
	//}

	//void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& value)
	//{
	//	GLint location = glGetUniformLocation(m_RendererID, name.c_str());
	//	glUniform2f(location, value.x, value.y);
	//}

	//void OpenGLShader::SetFloat(const std::string& name, float value)
	//{
	//	HZ_PROFILE_FUNCTION();

	//	UploadUniformFloat(name, value);
	//}

	//void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	//{
	//}


	//void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& value)
	//{
	//	GLint location = glGetUniformLocation(m_RendererID, name.c_str());
	//	glUniform3f(location, value.x, value.y, value.z);
	//}

	//void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& value)
	//{
	//	GLint location = glGetUniformLocation(m_RendererID, name.c_str());
	//	glUniform4f(location, value.x, value.y, value.z, value.w);
	//}

	//void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	//{
	//	GLint location = glGetUniformLocation(m_RendererID, name.c_str());
	//	glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	//}

	//void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	//{
	//	GLint location = glGetUniformLocation(m_RendererID, name.c_str());
	//	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	//}

}