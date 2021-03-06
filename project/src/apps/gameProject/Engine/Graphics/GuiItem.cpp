/// Internal Includes
#include "GuiItem.hpp"

#include "../Input/Input.hpp"

/// External Includes

/// Std Includes

namespace Engine {
	namespace Graphics {


		GuiItem::GuiItem(GuiInfo& info)
			: guiInfo(info)
			, absolutePosition()
			, screenRegion()
			, visible(false)
			, hasFocus(false)
		{
			relativePosition = glm::ivec2(0);
			size = glm::ivec2(1);
			anchorPoint = GuiAnchor::CENTER;
			uvCoords = glm::vec4(0.0F, 0.0F, 1.0F, 1.0F);
			zIndex = 0;
			hitTest = true;
			themeOverride = nullptr;
		}

		GuiItem::~GuiItem() {}

		bool GuiItem::isMouseInside() const {

			bool inside = false;
			if (true == hitTest) {
				if (visible) {
					Input::Input* in = Input::Input::GetInput();
					int x, y;
					in->getMousePos(x, y);
					inside = posInItem(x, y);
				}
			}
			return inside;
		}

		int GuiItem::getZIndex() const {
			return zIndex;
		}

		void GuiItem::setAnchorPoint(GuiAnchor anchor) {
			anchorPoint = anchor;
		}

		void GuiItem::setPosition(int x, int y) {
			relativePosition = glm::ivec2(x, y);
		}

		void GuiItem::setSize(int w, int h) {
			size = glm::ivec2(w, h);
		}

		void GuiItem::getSize(int &w, int &h) {
			w = size.x;
			h = size.y;
		}

		bool GuiItem::isVisible() const {
			return visible;
		}

		void GuiItem::setVisible(bool visibilityFlag) {
			visible = visibilityFlag;
		}

		void GuiItem::setZIndex(int _zIndex) {
			zIndex = _zIndex;
		}

		void GuiItem::setHitTest(bool enable) {
			hitTest = enable;
		}

		void GuiItem::updateRenderRegion(ScreenRegion region) {
			screenRegion = region;
		}

		void GuiItem::updateAbsoultePos(const int xOff, const int yOff, const int xSize, const int ySize) {

			const int ox = xOff;
			const int oy = yOff;

			if (size.x > xSize)
				size.x = xSize;
			if (size.y > ySize)
				size.y = ySize;

			int x = xSize;
			int y = ySize;

			glm::ivec2 realSize = size / 2;

			int aX = 0;
			int aY = 0;

			if (anchorPoint == GuiAnchor::CENTER) {
				x /= 2;
				y /= 2;

				aX = x - realSize.x;
				aY = y - realSize.y;

			} else if (anchorPoint == GuiAnchor::TOP) {
				x /= 2;
				y = 0;

				aX = x - realSize.x;
				aY = y;

			} else if (anchorPoint == GuiAnchor::BOTTOM) {
				x /= 2;

				aX = x - realSize.x;
				aY = y - 2 * realSize.y;

			} else if (anchorPoint == GuiAnchor::LEFT) {
				x = 0;
				y /= 2;

				aX = x;
				aY = y - realSize.y;

			} else if (anchorPoint == GuiAnchor::RIGHT) {
				y /= 2;

				aX = x - 2 * realSize.x;
				aY = y - realSize.y;

			} else if (anchorPoint == GuiAnchor::TOP_LEFT) {
				x = 0;
				y = 0;

				aX = x;
				aY = y;

			} else if (anchorPoint == GuiAnchor::TOP_RIGHT) {
				y = 0;

				aX = x - 2 * realSize.x;
				aY = y;

			} else if (anchorPoint == GuiAnchor::BOTTOM_RIGHT) {

				aX = x - 2 * realSize.x;
				aY = y - 2 * realSize.y;

			} else if (anchorPoint == GuiAnchor::BOTTOM_LEFT) {
				x = 0;

				aX = x - 2 * realSize.x;
				aY = y;
			}

			int mx = ox + relativePosition.x;
			int my = oy + relativePosition.y;

			aX += mx;
			aY += my;

			if (aX < ox) {
				aX = ox;
			}

			if (aY < oy) {
				aY = oy;
			}

			absolutePosition.x = aX;
			absolutePosition.y = aY;

		}
		bool GuiItem::isFocusable() const {
			return false;
		}

		bool GuiItem::hasFocusableItems() const {
			return false;
		}

		void GuiItem::update(float , GuiHitInfo& hitInfo, GuiItem* ) {
			// do nothing
			if (visible) {
				if (isMouseInside()) {
					hitInfo.mouseHit = true;
				}
			}
		}

		void GuiItem::render(glm::mat4 &, GuiShaderContainer& ) {
			// do nothing
		}

		void GuiItem::setThemeOverride(Theme::GuiTheme* theme) {
			themeOverride = theme;
		}

		void GuiItem::calculatePoints(glm::mat4 &positionMatrix) {

			const float uv1x = uvCoords[0];
			const float uv1y = uvCoords[1];

			const float uv2x = uvCoords[0];
			const float uv2y = uvCoords[3];

			const float uv3x = uvCoords[2];
			const float uv3y = uvCoords[3];

			const float uv4x = uvCoords[2];
			const float uv4y = uvCoords[1];

			const float x = float(absolutePosition.x);
			const float y = float(absolutePosition.y);

			glm::vec4 p0 = glm::vec4(x, y, uv1x, uv1y);
			glm::vec4 p1 = glm::vec4(x, y + size.y, uv2x, uv2y);
			glm::vec4 p2 = glm::vec4(x + size.x, y + size.y, uv3x, uv3y);
			glm::vec4 p3 = glm::vec4(x + size.x, y, uv4x, uv4y);

			positionMatrix[0] = p0;
			positionMatrix[1] = p1;
			positionMatrix[2] = p2;
			positionMatrix[3] = p3;

		}

		glm::mat4 GuiItem::genFromPosSize(glm::vec4 posSize) {

			glm::mat4 mat;

			const float uv1x = uvCoords[0];
			const float uv1y = uvCoords[1];

			const float uv2x = uvCoords[0];
			const float uv2y = uvCoords[3];

			const float uv3x = uvCoords[2];
			const float uv3y = uvCoords[3];

			const float uv4x = uvCoords[2];
			const float uv4y = uvCoords[1];

			const float x = float(posSize.x);
			const float y = float(posSize.y);

			glm::vec4 p0 = glm::vec4(x, y, uv1x, uv1y);
			glm::vec4 p1 = glm::vec4(x, y + posSize.w, uv2x, uv2y);
			glm::vec4 p2 = glm::vec4(x + posSize.z, y + posSize.w, uv3x, uv3y);
			glm::vec4 p3 = glm::vec4(x + posSize.z, y, uv4x, uv4y);

			mat[0] = p0;
			mat[1] = p1;
			mat[2] = p2;
			mat[3] = p3;
			return mat;
		}

		glm::vec4 GuiItem::clipRegion(glm::vec4 element, glm::vec4 region) {

			glm::vec4 newPos = element;

			if (element[0] < region[0])
				newPos[0] = region[0];
			if (element[1] < region[1])
				newPos[1] = region[1];
			if (element[2] > region[2])
				newPos[2] = region[2];
			if (element[3] > region[3])
				newPos[3] = region[3];

			return newPos;
		}

		bool GuiItem::posInItem(int x, int y) const {

			x -= absolutePosition.x;
			y -= absolutePosition.y;

			if ((x > 0 && x < size.x) &&
				(y > 0 && y < size.y)) {
				return true;
			}
			return false;
		}

		glm::vec4 GuiItem::positionAndSizeFromMatrix(const glm::mat4 &positionMatrix) {

			glm::vec4 p0 = positionMatrix[0];
			glm::vec4 p2 = positionMatrix[2];

			float x = p0.x;
			float y = p0.y;
			float w = p2.x - p0.x;
			float h = p2.y - p0.y;

			return glm::vec4(x, y, w, h);
		}
	}
}