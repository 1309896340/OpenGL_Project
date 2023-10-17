#include "line.h"

LineStructure lineManager;

void initLineDrawing(Shader* shader) {
	if(shader == nullptr)
		lineManager.shader = DefaultShader::getShader();
	else
		lineManager.shader = shader;
	glGenBuffers(1, &lineManager.vbo_line);
	glBindBuffer(GL_ARRAY_BUFFER, lineManager.vbo_line);
	glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), NULL, GL_STATIC_DRAW);
}
void showLines() {
	Shader& shader = *lineManager.shader;
	shader.use();
	shader["model"] = glm::mat4(1.0f);
	shader["modelBuffer"] = glm::mat4(1.0f);
	shader["isAuto"] = false;

	while (!lineManager.lines.empty()) {
		Line a = lineManager.lines.back();
		glBindBuffer(GL_ARRAY_BUFFER, lineManager.vbo_line);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6, &a.begin, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, NULL);
		glEnableVertexAttribArray(0);
		shader["ncolor"] = glm::vec4(a.color.x, a.color.y, a.color.z, a.color.w);
		glLineWidth(a.width);
		glDrawArrays(GL_LINES, 0, 2);
		glLineWidth(DEFAULT_LINE_WIDTH);
		lineManager.lines.pop_back();
	}
}
void drawLine(glm::vec3 begin, glm::vec3 end, glm::vec3 color, float width, Shader* shader) {
	Line a;
	a.begin = { begin.x,begin.y,begin.z };
	a.end = { end.x,end.y,end.z };
	a.color = { color.x,color.y,color.z,1.0f };
	a.width = width;

	lineManager.lines.push_back(a);
}
