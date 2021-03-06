
/// Internal Includes
#include "GuiWindow.hpp"
#include "../Graphics.hpp"
#include "../../Input/Input.hpp"

/// External Includes

/// Std Includes

/// Constants
const int GUI_WINDOW_TITLEBAR_HEIGHT = 30;

namespace Engine {
	namespace Graphics {
		namespace Gui {

			GuiWindow::GuiWindow(GuiInfo& info) : GuiItem(info) {

				setAnchorPoint(GuiAnchor::TOP_LEFT);

				titleBar = new Texture::Texture2D();
				closeButton = new Button(info);

				buttonTexture = new Texture::Texture2D();
				buttonHover = new Texture::Texture2D();
				buttonPress = new Texture::Texture2D();

				itemPanel = new Panel(info);

				itemPanel->setVisible(true);
				itemPanel->setAnchorPoint(GuiAnchor::TOP);
				itemPanel->setPosition(0, GUI_WINDOW_TITLEBAR_HEIGHT);

				titleBar->singleColor(0.2F, 0.2F, 0.2F, 1.0F);

				buttonTexture->singleColor(0.8F, 0.8F, 0.8F, 1.0F);
				buttonHover->singleColor(0.9F, 0.6F, 0.6F, 1.0F);
				buttonPress->singleColor(1.0F, 1.0F, 1.0F, 1.0F);

				closeButton->setVisible(true);
				closeButton->setPosition(-5, 5);

				closeButton->setSize(20, 20);
				closeButton->setAnchorPoint(GuiAnchor::TOP_RIGHT);
				closeButton->setText("X");

				subItems.push_back(closeButton);
				subItems.push_back(itemPanel);

				oldX = 0;
				oldY = 0;
				updating = false;
			}

			GuiWindow::~GuiWindow() {
				delete titleBar;
				delete closeButton;

				delete buttonTexture;
				delete buttonHover;
				delete buttonPress;

				delete itemPanel;
			}

			void GuiWindow::addGuiItem(GuiItem* guiItem) {
				itemPanel->addGuiItem(guiItem);
			}

			void GuiWindow::update(float dt, GuiHitInfo& hitInfo, GuiItem* currentFocus) {
				if (visible) {

					if (!closeButton->isMouseInside()) {
						Input::Input* in = Input::Input::GetInput();

						int x = 0;
						int y = 0;

						in->getMousePos(x, y);

						int mx, my;

						mx = x;
						my = y;

						x -= absolutePosition.x;
						y -= absolutePosition.y;

						bool inside = false;

						if (((x > 0 && x < size.x) &&
							(y > 0 && y < GUI_WINDOW_TITLEBAR_HEIGHT))
							|| (isMouseInside() && in->isKeyBindPressed(Input::KeyBindings[Input::KEYBIND_MOUSE_WHEEL_CLICK]))) {
							inside = true;
						}

						if (inside || updating) {

							if (in->isKeyBindPressed(Input::KeyBindings[Input::KEYBIND_MOUSE_L_CLICK]) || in->isKeyBindPressed(Input::KeyBindings[Input::KEYBIND_MOUSE_WHEEL_CLICK])) {

								int px = relativePosition.x;
								int py = relativePosition.y;

								px += (mx - oldX);
								py += (my - oldY);

								setPosition(px, py);

								updating = true;

							} else {
								updating = false;
							}

						}

						oldX = mx;
						oldY = my;

					}

					itemPanel->setSize(size.x, size.y - GUI_WINDOW_TITLEBAR_HEIGHT);

					if (closeButton->wasPressed()) {
						visible = false;
					}

					setAnchorPoint(GuiAnchor::TOP_LEFT);

					std::vector<GuiItem*>::reverse_iterator it = subItems.rbegin();
					std::vector<GuiItem*>::reverse_iterator eit = subItems.rend();

					for (it; it != eit; it++) {
						(*it)->updateAbsoultePos(absolutePosition.x, absolutePosition.y, size.x, size.y);
						(*it)->update(dt, hitInfo, currentFocus);
					}
				}
			}

			void GuiWindow::render(glm::mat4 &vpMatRef, GuiShaderContainer& shaderContainer) {
				if (visible) {

					calculatePoints(vpMatRef);
					glm::vec4 posAndSize = positionAndSizeFromMatrix(vpMatRef);

					int textureSlot = 0;

					// setup shader
					shaderContainer.guiElementShader->useShader();
					shaderContainer.guiElementShader->bindData(shaderContainer.elementVpMat, UniformDataType::UNI_MATRIX4X4, &shaderContainer.orthoMatrix);
					shaderContainer.guiElementShader->bindData(shaderContainer.elementTransformMat, UniformDataType::UNI_MATRIX4X4, &vpMatRef);
					shaderContainer.guiElementShader->bindData(shaderContainer.elementTexture, UniformDataType::UNI_INT, &textureSlot);

					shaderContainer.standardQuad->bind();

					shaderContainer.standardQuad->render();

					const ScissorInfo scinfo = gRenderEngine->pushScissorRegion((int)posAndSize.x, (int)posAndSize.y, (int)posAndSize.z, GUI_WINDOW_TITLEBAR_HEIGHT);

					titleBar->bind();
					shaderContainer.standardQuad->render();

					std::vector<GuiItem*>::iterator it = subItems.begin();
					std::vector<GuiItem*>::iterator eit = subItems.end();

					posAndSize = positionAndSizeFromMatrix(vpMatRef);
					// render all subitems
					for (it; it != eit; it++) {
						glm::mat4 cpy = vpMatRef;
						(*it)->render(cpy, shaderContainer);
					}
					gRenderEngine->popScissorRegion(scinfo);
				}
			}

		}
	}
}