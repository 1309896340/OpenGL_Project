#include "proj.h"

std::string readSource(std::string filename);
void loadShader(GLuint shader, std::string source);
GLuint loadProgram(std::string vertexSource, std::string fragmentSource);
GLuint loadProgramFromFile(std::string vertexSourcePath, std::string fragmentSourcePath);

std::ostream& operator<<(std::ostream& os, const glm::vec3& v);

void prepareVAO(const std::vector<vec3>& vertex, const std::vector<vec3>& normal, const std::vector<GLuint>& index, GLuint* VAO, GLuint* VBO, GLsizei* length);
