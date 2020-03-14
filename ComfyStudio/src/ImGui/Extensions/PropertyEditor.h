#pragma once
#include "ImGuiExtensions.h"
#include "InternalExtensions.h"

namespace ImGui
{
	namespace PropertyEditor
	{
		using ComponentFlags = uint8_t;
		enum ComponentFlagsEnum : ComponentFlags
		{
			ComponentFlags_None = 0,
			ComponentFlags_X = (1 << 0),
			ComponentFlags_Y = (1 << 1),
			ComponentFlags_Z = (1 << 2),
			ComponentFlags_W = (1 << 3),
		};

		constexpr const char* StringViewStart(std::string_view stringView) { return &stringView.front(); }
		constexpr const char* StringViewEnd(std::string_view stringView) { return &stringView.back() + 1; }

		namespace RAII
		{
			struct ID : NonCopyable
			{
				ID(int intID) { PushID(intID); };
				ID(const void* ptrID) { PushID(ptrID); };
				ID(std::string_view strID) { PushID(StringViewStart(strID), StringViewEnd(strID)); };
				~ID() { PopID(); };
			};

			struct ItemWidth : NonCopyable
			{
				ItemWidth(float width) { PushItemWidth(width); }
				~ItemWidth() { PopItemWidth(); }
			};

			struct PropertyValueColumns : NonCopyable
			{
				PropertyValueColumns() { Columns(2, nullptr, true); }
				~PropertyValueColumns() { Columns(1); }
			};

			struct TreeNode : NonCopyable
			{
				TreeNode(std::string_view label, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None, std::string_view valueLabel = "")
				{
					AlignTextToFramePadding();
					NoPushOnOpen = (flags & ImGuiTreeNodeFlags_NoTreePushOnOpen);
					IsOpen = WideTreeNodeEx(label.data(), flags);
					NextColumn();
					if (!valueLabel.empty())
					{
						AlignTextToFramePadding();
						TextUnformatted(StringViewStart(valueLabel), StringViewEnd(valueLabel));
					}
					NextColumn();
				}
				~TreeNode() { if (IsOpen && !NoPushOnOpen) TreePop(); }
				bool IsOpen, NoPushOnOpen;
				operator bool() { return IsOpen; }
			};
		}

		namespace Widgets
		{
			namespace Detail
			{
				constexpr const char* DummyLabel = "##Dummy";
				constexpr bool PerItemSeparation = true;

				inline void PerItemSeparator()
				{
					if (PerItemSeparation)
						Gui::Separator();
				}

				inline void PushItemDisabled()
				{
					PushItemFlag(ImGuiItemFlags_Disabled, true);
					PushStyleColor(ImGuiCol_Text, GImGui->Style.Colors[ImGuiCol_TextDisabled]);
				}

				inline void PopItemDisabled()
				{
					PopStyleColor();
					PopItemFlag();
				}

				template <typename T> struct ImGuiDataTypeLookup {};
				template<> struct ImGuiDataTypeLookup<int> { static constexpr ImGuiDataType Value = ImGuiDataType_S32; };
				template<> struct ImGuiDataTypeLookup<uint32_t> { static constexpr ImGuiDataType Value = ImGuiDataType_U32; };
				template<> struct ImGuiDataTypeLookup<float> { static constexpr ImGuiDataType Value = ImGuiDataType_Float; };

				template <typename T>
				inline bool DragText(std::string_view label, T& inOutValue, float speed, T* min, T* max, float width)
				{
					ImGuiWindow* window = GetCurrentWindow();
					const ImGuiID id = window->GetID(&inOutValue);

					vec2 labelSize = CalcTextSize(StringViewStart(label), StringViewEnd(label), true);
					if (width > 0.0f)
						labelSize.x = 0.0f;

					const auto frameBB = ImRect(window->DC.CursorPos, window->DC.CursorPos + vec2(width > 0.0f ? width : GetContentRegionAvailWidth(), labelSize.y + GImGui->Style.FramePadding.y * 2.0f));
					const auto totalBB = ImRect(frameBB.Min, frameBB.Max + vec2(labelSize.x > 0.0f ? GImGui->Style.ItemInnerSpacing.x + labelSize.x : 0.0f, 0.0f));

					ItemSize(totalBB, GImGui->Style.FramePadding.y);
					if (!ItemAdd(totalBB, id))
						return false;

					const bool hovered = ItemHoverable(frameBB, id);
					if (hovered && (GImGui->IO.MouseClicked[0] || GImGui->IO.MouseDoubleClicked[0]))
					{
						SetActiveID(id, window);
						SetFocusID(id, window);
						FocusWindow(window);
					}

					const bool valueChanged = Gui::DragBehavior(id, ImGuiDataTypeLookup<T>::Value, &inOutValue, speed, min, max, "", 1.0f, ImGuiDragFlags_None);
					if (valueChanged)
						MarkItemEdited(id);

					const bool isDragging = (GImGui->ActiveId == id);
					if (hovered || isDragging)
						SetMouseCursor(ImGuiMouseCursor_ResizeEW);

					RenderTextClipped(frameBB.Min, frameBB.Max, StringViewStart(label), StringViewEnd(label), nullptr, vec2(0.0f, 0.5f));
					return valueChanged;
				}

				inline bool TextButton(std::string_view label, bool& inOutValue)
				{
					auto window = GetCurrentWindow();
					const vec2 buttonSize = CalcItemSize(vec2(GetContentRegionAvailWidth(), CalcTextSize(StringViewStart(label), StringViewEnd(label)).y), 0.0f, 0.0f);

					bool hovered, held;
					bool pressed = ButtonBehavior(ImRect(window->DC.CursorPos, window->DC.CursorPos + buttonSize), window->GetID(&inOutValue), &hovered, &held);

					if (pressed)
						inOutValue = !inOutValue;

					TextUnformatted(StringViewStart(label), StringViewEnd(label));
					return pressed;
				}

				template <typename VecType, typename ValueType>
				inline bool InputVecDragPropertyBase(VecType& inOutValue, float dragSpeed, ValueType dragMin, ValueType dragMax, ComponentFlags disabledComponents)
				{
					static constexpr std::array<std::string_view, 4> componentLabels = { "  X  ", "  Y  ", "  Z  ", "  W  " };

					const float componentLabelWidth = CalcTextSize(StringViewStart(componentLabels.front()), StringViewEnd(componentLabels.front())).x;
					const float perComponentInputFloatWidth = glm::round((GetContentRegionAvailWidth() / static_cast<float>(VecType::length())) - componentLabelWidth);

					bool anyValueChanged = false;
					for (auto component = 0; component < VecType::length(); component++)
					{
						RAII::ID id(&inOutValue[component]);

						const bool disabled = (disabledComponents & (1 << component));
						if (disabled)
							PushItemDisabled();

						constexpr float dragSpeed = 1.0f;
						anyValueChanged |= DragText<ValueType>(componentLabels[component], inOutValue[component], dragSpeed, &dragMin, &dragMax, componentLabelWidth);
						SameLine(0.0f, 0.0f);

						const bool isLastComponent = ((component + 1) == VecType::length());

						RAII::ItemWidth width(isLastComponent ? (GetContentRegionAvailWidth() - 1.0f) : perComponentInputFloatWidth);

						if constexpr (std::is_floating_point<ValueType>::value)
							anyValueChanged |= Gui::InputFloat(Detail::DummyLabel, &inOutValue[component], 0.0f, 0.0f);
						else
							anyValueChanged |= Gui::InputInt(Detail::DummyLabel, &inOutValue[component], 0, 0);

						if (!isLastComponent)
							SameLine(0.0f, 0.0f);

						if (disabled)
							PopItemDisabled();
					}
					return anyValueChanged;
				}
			}

			inline void Separator()
			{
				if (!Detail::PerItemSeparation)
					Gui::Separator();
			}

			template <typename Func>
			inline bool TreeNode(std::string_view label, std::string_view valueLabel, ImGuiTreeNodeFlags flags, Func func)
			{
				Detail::PerItemSeparator();
				RAII::TreeNode treeNode(label, flags, valueLabel);
				if (treeNode)
					func();
				return treeNode.IsOpen;
			}

			template <typename Func>
			inline bool TreeNode(std::string_view label, ImGuiTreeNodeFlags flags, Func func)
			{
				Detail::PerItemSeparator();
				RAII::TreeNode treeNode(label, flags);
				if (treeNode)
					func();
				return treeNode.IsOpen;
			}

			template <typename Func>
			inline bool TreeNode(std::string_view label, Func func)
			{
				return TreeNode(label, ImGuiTreeNodeFlags_None, func);
			}

			template <typename PropertyFunc, typename ValueFunc>
			inline bool PropertyFuncValueFunc(PropertyFunc propertyGuiFunc, ValueFunc valueGuiFunc)
			{
				Detail::PerItemSeparator();
				AlignTextToFramePadding();
				bool anyValueChanged = propertyGuiFunc();
				NextColumn();

				AlignTextToFramePadding();
				anyValueChanged |= valueGuiFunc();
				NextColumn();

				return anyValueChanged;
			}

			template <typename Func>
			inline bool PropertyLabelValueFunc(std::string_view label, Func valueGuiFunc)
			{
				RAII::ID id(label);
				return PropertyFuncValueFunc([&]
				{
					TextUnformatted(StringViewStart(label), StringViewEnd(label));
					return false;
				}, [&]
				{
					return valueGuiFunc();
				});
			}

			inline bool Input(std::string_view label, float& inOutValue, float dragSpeed = 1.0f, float dragMin = 0.0f, float dragMax = 0.0f)
			{
				RAII::ID id(label);
				return PropertyFuncValueFunc([&]
				{
					return Detail::DragText(label, inOutValue, dragSpeed, &dragMin, &dragMax, 0.0f);
				}, [&]
				{
					RAII::ItemWidth width(-1.0f);
					return Gui::InputFloat(Detail::DummyLabel, &inOutValue, dragSpeed, dragSpeed * 10.0f);
				});
			}

			inline bool Input(std::string_view label, vec2& inOutValue, float dragSpeed = 1.0f, float dragMin = 0.0f, float dragMax = 0.0f, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecDragPropertyBase<vec2, vec2::value_type>(inOutValue, dragSpeed, dragMin, dragMax, disabledComponents); });
			}

			inline bool Input(std::string_view label, vec3& inOutValue, float dragSpeed = 1.0f, float dragMin = 0.0f, float dragMax = 0.0f, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecDragPropertyBase<vec3, vec3::value_type>(inOutValue, dragSpeed, dragMin, dragMax, disabledComponents); });
			}

			inline bool Input(std::string_view label, vec4& inOutValue, float dragSpeed = 1.0f, float dragMin = 0.0f, float dragMax = 0.0f, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecDragPropertyBase<vec4, vec4::value_type>(inOutValue, dragSpeed, dragMin, dragMax, disabledComponents); });
			}

			inline bool Input(std::string_view label, int& inOutValue, int step = 1, int fastStep = 10)
			{
				using InputType = std::remove_reference<decltype(inOutValue)>::type;

				RAII::ID id(label);
				return PropertyFuncValueFunc([&]
				{
					return Detail::DragText<InputType>(label, inOutValue, 0.1f, nullptr, nullptr, 0.0f);
				}, [&]
				{
					RAII::ItemWidth width(-1.0f);
					return Gui::InputInt(Detail::DummyLabel, &inOutValue, step, fastStep);
				});
			}

			inline bool Input(std::string_view label, uint32_t& inOutValue, uint32_t step = 1, uint32_t fastStep = 10)
			{
				using InputType = std::remove_reference<decltype(inOutValue)>::type;

				RAII::ID id(label);
				return PropertyFuncValueFunc([&]
				{
					return Detail::DragText<InputType>(label, inOutValue, 0.1f, nullptr, nullptr, 0.0f);
				}, [&]
				{
					RAII::ItemWidth width(-1.0f);
					return Gui::InputScalar(Detail::DummyLabel, Detail::ImGuiDataTypeLookup<InputType>::Value, &inOutValue, &step, &fastStep);
				});
			}

			inline bool InputHex(std::string_view label, uint32_t& inOutValue)
			{
				using InputType = std::remove_reference<decltype(inOutValue)>::type;

				RAII::ID id(label);
				return PropertyFuncValueFunc([&]
				{
					return Detail::DragText<InputType>(label, inOutValue, 0.1f, nullptr, nullptr, 0.0f);
				}, [&]
				{
					uint32_t step = 1, fastStep = 1;

					RAII::ItemWidth width(-1.0f);
					return Gui::InputScalar(Detail::DummyLabel, Detail::ImGuiDataTypeLookup<InputType>::Value, &inOutValue, &step, &fastStep, "%X", ImGuiInputTextFlags_CharsHexadecimal);
				});
			}

			inline bool Input(std::string_view label, ivec2& inOutValue, float dragSpeed = 1.0f, int dragMin = 0, int dragMax = 0, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecDragPropertyBase<ivec2, ivec2::value_type>(inOutValue, dragSpeed, dragMin, dragMax, disabledComponents); });
			}

			inline bool Input(std::string_view label, ivec3& inOutValue, float dragSpeed = 1.0f, int dragMin = 0, int dragMax = 0, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecDragPropertyBase<ivec3, ivec3::value_type>(inOutValue, dragSpeed, dragMin, dragMax, disabledComponents); });
			}

			inline bool Input(std::string_view label, ivec4& inOutValue, float dragSpeed = 1.0f, int dragMin = 0, int dragMax = 0, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecDragPropertyBase<ivec4, ivec4::value_type>(inOutValue, dragSpeed, dragMin, dragMax, disabledComponents); });
			}

			inline bool Input(std::string_view label, char* inOutBuffer, size_t bufferSize, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { RAII::ItemWidth width(-1.0f); return Gui::InputText(Detail::DummyLabel, inOutBuffer, bufferSize, flags); });
			}

			inline bool Checkbox(std::string_view label, bool& inOutValue)
			{
				RAII::ID id(label);
				return PropertyFuncValueFunc([&]
				{
					return Detail::TextButton(label, inOutValue);
				}, [&]
				{
					return Gui::Checkbox(Detail::DummyLabel, &inOutValue);
				});
			}

			inline bool CheckboxFlags(std::string_view label, uint32_t& inOutValue, uint32_t flagsMask)
			{
				bool isChecked = ((inOutValue & flagsMask) == flagsMask);
				if (bool valueChanged = Checkbox(label, isChecked); valueChanged)
				{
					if (isChecked)
						inOutValue |= flagsMask;
					else
						inOutValue &= ~flagsMask;
					return true;
				}
				return false;
			}

			// NOTE: IndexToStringFunc should handle index validation itself (range checking for arrays), nullptr returns will be skipped
			template <typename IndexToStringFunc>
			inline bool Combo(std::string_view label, int& inOutIndex, int startRange, int endRange, ImGuiComboFlags flags, IndexToStringFunc indexToString)
			{
				return PropertyLabelValueFunc(label, [&]
				{
					bool valueChanged = false;

					const char* previewValue = indexToString(inOutIndex);
					if (InternalVariableWidthBeginCombo(Detail::DummyLabel, (previewValue == nullptr) ? "" : previewValue, flags, GetContentRegionAvailWidth()))
					{
						for (int i = startRange; i < endRange; i++)
						{
							if (const char* itemLabel = indexToString(i); itemLabel != nullptr)
							{
								const bool isSelected = (i == inOutIndex);
								if (Selectable(itemLabel, isSelected))
								{
									inOutIndex = i;
									valueChanged = true;
								}
								if (isSelected)
									SetItemDefaultFocus();
							}
						}
						EndCombo();
					}

					return valueChanged;
				});
			}

			template <typename IndexToStringFunc>
			inline bool Combo(std::string_view label, int& inOutIndex, int startRange, int endRange, IndexToStringFunc indexToString)
			{
				return Combo(label, inOutIndex, startRange, endRange, ImGuiComboFlags_None, indexToString);
			}

			template <typename EnumType, size_t ArraySize>
			inline bool Combo(std::string_view label, EnumType& inOutEnum, const std::array<const char*, ArraySize>& nameLookup, ImGuiComboFlags flags = ImGuiComboFlags_None)
			{
				static_assert(sizeof(EnumType) <= sizeof(int));

				int tempIndex = static_cast<int>(inOutEnum);
				if (Combo(label, tempIndex, 0, static_cast<int>(ArraySize), flags, [&](int index) { return (index >= 0 && index < nameLookup.size()) ? nameLookup[index] : nullptr; }))
				{
					inOutEnum = static_cast<EnumType>(tempIndex);
					return true;
				}
				return false;
			}

			inline bool ColorEdit(std::string_view label, vec3& inOutValue, ImGuiColorEditFlags flags = ImGuiColorEditFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { RAII::ItemWidth width(-1.0f); return Gui::ColorEdit3(Detail::DummyLabel, glm::value_ptr(inOutValue), flags); });
			}

			inline bool ColorEdit(std::string_view label, vec4& inOutValue, ImGuiColorEditFlags flags = ImGuiColorEditFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { RAII::ItemWidth width(-1.0f); return Gui::ColorEdit4(Detail::DummyLabel, glm::value_ptr(inOutValue), flags); });
			}
		}
	}
}

namespace GuiProperty = Gui::PropertyEditor::Widgets;
namespace GuiPropertyRAII = Gui::PropertyEditor::RAII;
