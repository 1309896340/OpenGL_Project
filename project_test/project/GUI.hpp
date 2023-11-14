#include "proj.h"

#ifdef TEST_OPENGL

#ifndef __WIND_GUI
#define __WIND_GUI

#include "proj.h"
#include "InputManager.hpp"
#include "Geometry.hpp"
#include "Wheat.hpp"

#include  "imgui.h"
#include  "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"


//extern StatusInfo status;
//extern Leaf *leaf;

#define STR_BUFFER_N 100

class GUI {
private:
	GLFWwindow* window{ nullptr };
public:
	GUI(GLFWwindow* window) :window(window) {

		IMGUI_CHECKVERSION();

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;		// 启用键盘控制
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;	// 启用手柄控制
		ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 30.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
		ImGui::GetIO().FontDefault = font;

		bool flag = ImGui_ImplGlfw_InitForOpenGL(window, true);
		flag &= ImGui_ImplOpenGL3_Init();
		if (!flag) {
			std::cout << "ImGui init failed!" << flag << std::endl;
			exit(10);
		}
	}
	~GUI() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	void update() {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGui::ShowDemoWindow();
		// 绘制自定义的控件

		InputManager* ctx = reinterpret_cast<InputManager*>(glfwGetWindowUserPointer(window));
		Camera* camera = ctx->getCamera();
		Scene* scene = ctx->getScene();
		map<Geometry*, GeometryRenderInfo> objs = scene->getObjects();


		char showText[STR_BUFFER_N];

		ImGui::Begin(u8"交互窗口");

		ImGui::SetNextItemOpen(true);
		if (ImGui::CollapsingHeader(u8"相机属性")) {
			vec3 tmp;
			tmp = camera->getPosition();
			snprintf(showText, STR_BUFFER_N, u8"相机位置：(%.2f, %.2f, %.2f)", tmp.x, tmp.y, tmp.z);
			ImGui::Text(showText);
			snprintf(showText, STR_BUFFER_N, u8"航向角：%.1f°  俯仰角：%.1f°  翻滚角：%.1f°", camera->getYaw() * 180.0f / PI, camera->getPitch() * 180.0f / PI, camera->getRoll() * 180.0f / PI);
			ImGui::Text(showText);
			ImGui::SliderFloat(u8"FOV", &(camera->getFov()), 10.0f, 80.0f);
			//float a[3];
			//if (ImGui::SliderFloat3(u8"不知道什么滑块", a, 1.0, 3.0f)) {
			//	cout << "触发" << endl;
			//}

		}

		static int item_current = 0;		// 当前选中的几何体的索引
		unsigned int objNum = 0;		// 几何体数量
		const char** objNames = nullptr;		// 几何体名称列表

		ImGui::SetNextItemOpen(true);
		if (ImGui::CollapsingHeader(u8"对象管理")) {
			objNum = (unsigned int)objs.size();
			objNames = (const char**)new char* [objNum];

			unsigned idx = 0;
			for (auto ptr = objs.begin(); ptr != objs.end(); ptr++, idx++) {
				string& gname = ptr->first->getName();		// 这里使用的引用，因此Geometry中name的修改会直接反映到这里
				objNames[idx] = gname.c_str();
			}
			ImGui::Combo(u8"对象列表", &item_current, (const char**)objNames, objNum);
			// ===========显示选中项的属性===============
			string selectedGeometryName = objNames[item_current];
			auto ptr = std::find_if(objs.begin(), objs.end(), [&selectedGeometryName](const std::pair<Geometry*, GeometryRenderInfo> item) {
				return item.first->getName() == string(selectedGeometryName);
				// 不确定这里的比较是否正确，假定string正确重载了==，同时假定char数组也如预期的那样转换成string
				});
			if (ptr != objs.end()) {
				std::for_each(objs.begin(), objs.end(), [scene](std::pair<Geometry*, GeometryRenderInfo> item) {		// 重置shader
					item.second.shader = scene->shaders["default"];		// 假定原本使用的shader就是default
					scene->setShader(item.first, scene->shaders["default"]);
					});
				scene->setShader(ptr->first, scene->shaders["wired"]);		// 对选中对象使用线框shader


				Geometry* obj = ptr->first;
				// Geometry信息
				// 需要先将 model 取出，然后将其分解为平移、旋转、缩放三个量
				mat4 model = obj->getLocal2WorldMatrix();
				vec3 _scale, translation, skew;
				quat orientation;
				vec4 perspective;

				ImGui::SeparatorText("Geometry");
				if (ImGui::TreeNode("Global")) {
					glm::decompose(model, _scale, orientation, translation, skew, perspective);
					orientation = glm::conjugate(orientation);

					snprintf(showText, STR_BUFFER_N, u8"位置：(%.2f, %.2f, %.2f)", translation.x, translation.y, translation.z);
					ImGui::Text(showText);
					snprintf(showText, STR_BUFFER_N, u8"旋转：(%.2f, %.2f, %.2f, %.2f)", orientation.x, orientation.y, orientation.z, orientation.w);	// 用四元数表示旋转
					ImGui::Text(showText);
					snprintf(showText, STR_BUFFER_N, u8"缩放：(%.2f, %.2f, %.2f)", _scale.x, _scale.y, _scale.z);
					ImGui::Text(showText);

					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Local")) {
					vec3& posVal = obj->model.getPosition();
					quat& rotVal = obj->model.getRotation();
					vec3& scaleVal = obj->model.getScale();
					vec3 raxis = glm::axis(rotVal);
					float rangle = glm::angle(rotVal) * 180.0f / PI;
					ImGui::SliderFloat3(u8"位置", value_ptr(posVal), -5.0f, 5.0f);
					ImGui::SliderFloat3(u8"缩放", value_ptr(scaleVal), 0.01f, 10.0f);
					// 暂时不知道如何处理旋转的交互，如果使用SliderFloat3控制旋转轴，则需要将其限制在单位球面上，否则会出现奇怪的旋转
					// 而且从这里通过修改父节点的scale可以看到子节点的scale也会跟着变化，这是不对的，应当只会造成子节点偏移位置的变化
					// 或者不应当将缩放考虑进偏移矩阵的计算，换应当只考虑平移和旋转，或者只考虑均匀缩放
					//bool c1 = ImGui::SliderFloat3(u8"旋转轴", value_ptr(raxis), 0.0f, 1.0f);
					snprintf(showText, STR_BUFFER_N, u8"旋转轴：(%.2f, %.2f, %.2f)", raxis.x, raxis.y, raxis.z);	// 用四元数表示旋转
					ImGui::Text(showText);
					if (ImGui::SliderFloat(u8"旋转角", &rangle, 0.0f, 360.0f)) {
						rotVal = glm::angleAxis(rangle * PI / 180.0f, normalize(raxis));
					}
					ImGui::TreePop();
				}
				if (ImGui::TreeNode("Offset")) {
					// 偏移矩阵
					vec3& posVal = obj->offset.getPosition();
					quat& rotVal = obj->offset.getRotation();
					vec3 raxis = glm::axis(rotVal);
					float rangle = glm::angle(rotVal) * 180.0f / PI;
					ImGui::SliderFloat3(u8"位置", value_ptr(posVal), -5.0f, 5.0f);
					snprintf(showText, STR_BUFFER_N, u8"旋转轴：(%.2f, %.2f, %.2f)", raxis.x, raxis.y, raxis.z);	// 用四元数表示旋转
					ImGui::Text(showText);
					if (ImGui::SliderFloat(u8"旋转角", &rangle, 0.0f, 360.0f)) {
						rotVal = glm::angleAxis(rangle * PI / 180.0f, normalize(raxis));
					}

					ImGui::TreePop();
				}
				// 子类信息
				switch (obj->getType()) {
					Leaf* leaf;
					LeafMesh* leafmesh;
					Cube* cube;
					Sphere* sphere;
					Cylinder* cylinder;
					Cone* cone;

				case LEAF:
					ImGui::SeparatorText("Leaf");
					leaf = static_cast<Leaf*>(obj);
					leafmesh = static_cast<LeafMesh*>(leaf->getMeshes()[0]);
					if (ImGui::SliderFloat(u8"叶长", &(leafmesh->height), 1.0f, 4.5f)) {
						leafmesh->setChangeFlag();
					}
					if (ImGui::SliderFloat(u8"叶宽因子", &(leafmesh->theta), 0.0f, 0.5f)) {
						leafmesh->setChangeFlag();
					}
					if (ImGui::SliderFloat(u8"茎叶夹角", &(leafmesh->SLAngle), 0.1f, 120.0f)) {
						leafmesh->setChangeFlag();
					}
					if (ImGui::SliderFloat(u8"弯曲系数", &(leafmesh->k), 0.0f, 30.0f)) {
						leafmesh->setChangeFlag();
					}
					if (ImGui::SliderFloat(u8"主叶脉扭曲角", &(leafmesh->MVAngle), -540.0f, 540.0f)) {
						leafmesh->setChangeFlag();
					}
					break;
				case CUBE:
					ImGui::SeparatorText("Cube");
					cube = static_cast<Cube*>(obj);
					snprintf(showText, STR_BUFFER_N, u8"长度：%.2f  宽度：%.2f  高度：%.2f", cube->getXLength(), cube->getYLength(), cube->getZLength());
					ImGui::Text(showText);
					break;
				case SPHERE:
					ImGui::SeparatorText("Sphere");
					sphere = static_cast<Sphere*>(obj);
					snprintf(showText, STR_BUFFER_N, u8"半径：%.2f", sphere->getRadius());
					ImGui::Text(showText);
					break;
				case CYLINDER:
					ImGui::SeparatorText("Cylinder");
					cylinder = static_cast<Cylinder*>(obj);
					snprintf(showText, STR_BUFFER_N, u8"半径：%.2f  高度：%.2f", cylinder->getRadius(), cylinder->getHeight());
					ImGui::Text(showText);
					break;
				case CONE:
					ImGui::SeparatorText("Cone");
					cone = static_cast<Cone*>(obj);
					snprintf(showText, STR_BUFFER_N, u8"半径：%.2f  高度：%.2f", cone->getRadius(), cone->getHeight());
					ImGui::Text(showText);
					break;
				}
			}
		}


		//ImGui::ShowDemoWindow();

		delete[] objNames;
		ImGui::End();
	}
	void render() {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
};

#endif
#endif