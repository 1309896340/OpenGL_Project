#pragma once
#include "proj.h"


std::string readSource(std::string filename);
void updateUniformMatrix4fv(GLuint program, const std::string& varname, const glm::mat4& m);
void updateUniformVector3fv(GLuint program, const std::string& varname, const glm::vec3& v);
void updateUniformVector4fv(GLuint program, const std::string& varname, const glm::vec4& v);

void loadShader(GLuint shader, std::string source);
GLuint loadProgram(std::string vertexSource, std::string fragmentSource);
