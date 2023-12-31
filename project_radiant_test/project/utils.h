#ifndef _WIND_UTILS
#define _WIND_UTILS

#include "proj.h"

std::string readSource(std::string filename);
void loadShader(GLuint shader, std::string source);
GLuint loadProgram(std::string vertexSource, std::string fragmentSource);
GLuint loadProgramFromFile(std::string vertexSourcePath, std::string fragmentSourcePath);

std::ostream& operator<<(std::ostream& os, const glm::vec3& v);
std::ostream& operator<<(std::ostream& os, const glm::mat4& m);

void prepareVAO(const std::vector<vec3>& vertex, const std::vector<vec3>& normal, const std::vector<GLuint>& index, GLuint* VAO, GLuint* VBO, GLsizei* length);
void updateVertexPosition(GLuint VAO, GLuint VBO, std::vector<vec3> vertex);

GLFWwindow* GLFWinit();

glm::vec3 toGlm(const vec3& v);
glm::vec4 toGlm(const vec4& v);

vec3 toVec(const glm::vec3& v);
vec4 toVec(const glm::vec4& v);

extern void framebuff_size_callback(GLFWwindow* window, int width, int height);
extern void mouse_botton_callback(GLFWwindow* window, int button, int action, int mods);
extern void mouse_callback(GLFWwindow* window, double xpos, double ypos);
extern void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);


#endif