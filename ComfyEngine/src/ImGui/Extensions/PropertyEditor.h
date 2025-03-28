#pragma once
#include "ImGuiExtensions.h"
#include "InternalExtensions.h"
#include <optional>

namespace ImGui
{
	namespace PropertyEditor
	{
		using ComponentFlags = u8;
		enum ComponentFlagsEnum : ComponentFlags
		{
			ComponentFlags_None = 0,
			ComponentFlags_X = (1 << 0),
			ComponentFlags_Y = (1 << 1),
			ComponentFlags_Z = (1 << 2),
			ComponentFlags_W = (1 << 3),
		};

		namespace RAII
		{
			struct ID : NonCopyable
			{
				ID(i32 intID) { PushID(intID); }
				ID(const void* ptrID) { PushID(ptrID); }
				ID(std::string_view strID) { PushID(StringViewStart(strID), StringViewEnd(strID)); }
				~ID() { PopID(); }
			};

			struct ItemWidth : NonCopyable
			{
				ItemWidth(f32 width) { PushItemWidth(width); }
				~ItemWidth() { PopItemWidth(); }
			};

			// TODO: This all needs to be reworked, frame padding is all fucked up with the old columns. Hopefully this can be fixed using Tables 
			struct Columns : NonCopyable
			{
				Columns(i32 columnsCount, const char* id, bool border)
				{
					if (auto previousColumns = GetCurrentWindow()->DC.CurrentColumns; previousColumns == nullptr)
						Previous = { nullptr, 1, false };
					else
						Previous = { nullptr, previousColumns->Count, (previousColumns->Flags & ImGuiOldColumnFlags_NoBorder) == 0 };

					ImGui::Columns(columnsCount, id, border);
				}
				~Columns()
				{
					ImGui::Columns(Previous.Count, Previous.ID, Previous.Border);
				}
				struct ColumnData
				{
					const char* ID;
					i32 Count;
					bool Border;
				} Previous;
			};

			struct PropertyValueColumns : NonCopyable
			{
				Columns Columns { 2, nullptr, true };
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
						ImGui::Separator();
				}
			}

			template <typename Func>
			bool TreeNode(std::string_view label, std::string_view valueLabel, ImGuiTreeNodeFlags flags, Func func)
			{
				Detail::PerItemSeparator();
				RAII::TreeNode treeNode(label, flags, valueLabel);
				if (treeNode)
					func();
				return treeNode.IsOpen;
			}

			template <typename Func>
			bool TreeNode(std::string_view label, ImGuiTreeNodeFlags flags, Func func)
			{
				Detail::PerItemSeparator();
				RAII::TreeNode treeNode(label, flags);
				if (treeNode)
					func();
				return treeNode.IsOpen;
			}

			template <typename Func>
			bool TreeNode(std::string_view label, Func func)
			{
				return TreeNode(label, ImGuiTreeNodeFlags_None, func);
			}

			template <typename PropertyFunc, typename ValueFunc>
			bool PropertyFuncValueFunc(PropertyFunc propertyGuiFunc, ValueFunc valueGuiFunc)
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
			bool PropertyLabelValueFunc(std::string_view label, Func valueGuiFunc)
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

			namespace Detail
			{
				namespace TypeLookup
				{
					template <typename T>
					struct DataType
					{
					};
					template<>
					struct DataType<i32>
					{
						static constexpr const char* Format = nullptr;
						static constexpr ImGuiDataType TypeEnum = ImGuiDataType_S32;
						static constexpr ImGuiInputTextFlags InputTextFlags = ImGuiInputTextFlags_None;
					};
					template<>
					struct DataType<u32>
					{
						static constexpr const char* Format = nullptr;
						static constexpr ImGuiDataType TypeEnum = ImGuiDataType_U32;
						static constexpr ImGuiInputTextFlags InputTextFlags = ImGuiInputTextFlags_None;
					};
					template<>
					struct DataType<f32>
					{
						static constexpr const char* Format = "%.3f";
						static constexpr ImGuiDataType TypeEnum = ImGuiDataType_Float;
						static constexpr ImGuiInputTextFlags InputTextFlags = ImGuiInputTextFlags_CharsScientific;
					};
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

				template <typename T>
				bool DragTextT(std::string_view label, T& inOutValue, f32 speed, T* min, T* max, f32 width)
				{
					ImGuiWindow* window = GetCurrentWindow();
					const ImGuiID id = window->GetID(&inOutValue);

					vec2 labelSize = CalcTextSize(StringViewStart(label), StringViewEnd(label), true);
					if (width > 0.0f)
						labelSize.x = 0.0f;

					const auto frameBB = ImRect(window->DC.CursorPos, window->DC.CursorPos + vec2(width > 0.0f ? width : GetContentRegionAvail().x, labelSize.y + GImGui->Style.FramePadding.y * 2.0f));
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

					const bool valueChanged = ImGui::DragBehavior(id, TypeLookup::DataType<T>::TypeEnum, &inOutValue, speed, min, max, "", ImGuiSliderFlags_None);
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
					const vec2 buttonSize = CalcItemSize(vec2(GetContentRegionAvail().x, CalcTextSize(StringViewStart(label), StringViewEnd(label)).y), 0.0f, 0.0f);

					bool hovered, held;
					bool pressed = ButtonBehavior(ImRect(window->DC.CursorPos, window->DC.CursorPos + buttonSize), window->GetID(&inOutValue), &hovered, &held);

					if (pressed)
						inOutValue = !inOutValue;

					TextUnformatted(StringViewStart(label), StringViewEnd(label));
					return pressed;
				}

				template <typename ValueType>
				bool InputVec1DragBaseValueFunc(ValueType& inOutValue, f32 dragSpeed, std::optional<glm::vec<2, ValueType>> dragRange, const char* format = nullptr)
				{
					RAII::ItemWidth width(-1.0f);

					constexpr bool isFloat = std::is_floating_point<ValueType>::value;
					const ValueType step = isFloat ? static_cast<ValueType>(dragSpeed) : Comfy::Max(static_cast<ValueType>(1), static_cast<ValueType>(dragSpeed));
					const ValueType fastStep = static_cast<ValueType>(step * static_cast<ValueType>(10));

					using Lookup = TypeLookup::DataType<ValueType>;
					const bool valueChanged = ImGui::InputScalar(Detail::DummyLabel, Lookup::TypeEnum, &inOutValue, &step, &fastStep, (format != nullptr) ? format : Lookup::Format, Lookup::InputTextFlags);

					if (valueChanged && dragRange.has_value())
						inOutValue = Comfy::Clamp(inOutValue, dragRange->x, dragRange->y);

					return valueChanged;
				}

				template <typename ValueType>
				bool InputVec1DragBase(std::string_view label, ValueType& inOutValue, f32 dragSpeed, std::optional<glm::vec<2, ValueType>> dragRange, const char* format = nullptr)
				{
					RAII::ID id(label);
					return PropertyFuncValueFunc([&]
					{
						return Detail::DragTextT<ValueType>(label, inOutValue, dragSpeed,
							dragRange.has_value() ? &dragRange->x : nullptr,
							dragRange.has_value() ? &dragRange->y : nullptr,
							0.0f);
					}, [&]
					{
						return InputVec1DragBaseValueFunc(inOutValue, dragSpeed, dragRange, format);
					});
				}

				template <typename VecType, typename ValueType>
				bool InputVecNDragPropertyBase(VecType& inOutValue, f32 dragSpeed, std::optional<glm::vec<2, ValueType>> dragRange, ComponentFlags disabledComponents)
				{
					static constexpr std::array<std::string_view, 4> componentLabels = { "  X  ", "  Y  ", "  Z  ", "  W  " };

					const f32 componentLabelWidth = CalcTextSize(StringViewStart(componentLabels.front()), StringViewEnd(componentLabels.front())).x;
					const f32 perComponentInputFloatWidth = glm::round((GetContentRegionAvail().x / static_cast<f32>(VecType::length())) - componentLabelWidth);

					bool anyValueChanged = false;
					for (auto component = 0; component < VecType::length(); component++)
					{
						RAII::ID id(&inOutValue[component]);

						const bool disabled = (disabledComponents & (1 << component));
						if (disabled)
							PushItemDisabled();

						anyValueChanged |= DragTextT<ValueType>(componentLabels[component], inOutValue[component], dragSpeed,
							dragRange.has_value() ? &dragRange->x : nullptr,
							dragRange.has_value() ? &dragRange->y : nullptr,
							componentLabelWidth);
						SameLine(0.0f, 0.0f);

						const bool isLastComponent = ((component + 1) == VecType::length());
						RAII::ItemWidth width(isLastComponent ? (GetContentRegionAvail().x - 1.0f) : perComponentInputFloatWidth);

						using Lookup = TypeLookup::DataType<ValueType>;
						anyValueChanged |= ImGui::InputScalar(Detail::DummyLabel, Lookup::TypeEnum, &inOutValue[component], nullptr, nullptr, Lookup::Format, Lookup::InputTextFlags);

						if (!isLastComponent)
							SameLine(0.0f, 0.0f);

						if (disabled)
							PopItemDisabled();
					}
					return anyValueChanged;
				}

				inline bool InputFractionBase(ivec2& inOutValue, std::optional<ivec2> valueRange, ComponentFlags disabledComponents)
				{
					using VecType = ivec2;
					using ValueType = ivec2::value_type;

					constexpr std::string_view divisionLabel = " / ";
					const f32 divisionLabelWidth = CalcTextSize(StringViewStart(divisionLabel), StringViewEnd(divisionLabel)).x;
					const f32 perComponentInputFloatWidth = glm::floor(((GetContentRegionAvail().x - divisionLabelWidth) / static_cast<f32>(VecType::length())));

					bool anyValueChanged = false;
					for (auto component = 0; component < VecType::length(); component++)
					{
						RAII::ID id(&inOutValue[component]);

						const bool disabled = (disabledComponents & (1 << component));
						if (disabled)
							PushItemDisabled();

						const bool isLastComponent = ((component + 1) == VecType::length());
						RAII::ItemWidth width(isLastComponent ? (GetContentRegionAvail().x - 1.0f) : perComponentInputFloatWidth);

						using Lookup = TypeLookup::DataType<ValueType>;
						if (ImGui::InputScalar(Detail::DummyLabel, Lookup::TypeEnum, &inOutValue[component], nullptr, nullptr, Lookup::Format, Lookup::InputTextFlags))
						{
							if (valueRange.has_value())
								inOutValue[component] = Comfy::Clamp(inOutValue[component], valueRange->x, valueRange->y);

							anyValueChanged = true;
						}

						if (!isLastComponent)
						{
							SameLine(0.0f, 0.0f);
							TextUnformatted(StringViewStart(divisionLabel), StringViewEnd(divisionLabel));
							SameLine(0.0f, 0.0f);
						}

						if (disabled)
							PopItemDisabled();
					}
					return anyValueChanged;
				}
			}

			inline bool Input(std::string_view label, f32& inOutValue, f32 dragSpeed = 1.0f, std::optional<vec2> dragRange = {}, const char* format = nullptr)
			{
				return Detail::InputVec1DragBase(label, inOutValue, dragSpeed, dragRange, format);
			}

			inline bool Input(std::string_view label, vec2& inOutValue, f32 dragSpeed = 1.0f, std::optional<vec2> dragRange = {}, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecNDragPropertyBase<vec2, vec2::value_type>(inOutValue, dragSpeed, dragRange, disabledComponents); });
			}

			inline bool Input(std::string_view label, vec3& inOutValue, f32 dragSpeed = 1.0f, std::optional<vec2> dragRange = {}, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecNDragPropertyBase<vec3, vec3::value_type>(inOutValue, dragSpeed, dragRange, disabledComponents); });
			}

			inline bool Input(std::string_view label, vec4& inOutValue, f32 dragSpeed = 1.0f, std::optional<vec2> dragRange = {}, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecNDragPropertyBase<vec4, vec4::value_type>(inOutValue, dragSpeed, dragRange, disabledComponents); });
			}

			inline bool Input(std::string_view label, i32& inOutValue, f32 dragSpeed = 1.0f, std::optional<ivec2> dragRange = {}, const char* format = nullptr)
			{
				return Detail::InputVec1DragBase(label, inOutValue, dragSpeed, dragRange, format);
			}

			inline bool Input(std::string_view label, u32& inOutValue, f32 dragSpeed = 1.0f, std::optional<uvec2> dragRange = {}, const char* format = nullptr)
			{
				return Detail::InputVec1DragBase(label, inOutValue, dragSpeed, dragRange, format);
			}

			inline bool InputHex(std::string_view label, u32& inOutValue)
			{
				using InputType = std::remove_reference<decltype(inOutValue)>::type;

				RAII::ID id(label);
				return PropertyFuncValueFunc([&]
				{
					return Detail::DragTextT<InputType>(label, inOutValue, 0.1f, nullptr, nullptr, 0.0f);
				}, [&]
				{
					constexpr u32 step = 1, fastStep = 16;
					RAII::ItemWidth width(-1.0f);
					return ImGui::InputScalar(Detail::DummyLabel, Detail::TypeLookup::DataType<InputType>::TypeEnum, &inOutValue, &step, &fastStep, "%08X", ImGuiInputTextFlags_CharsHexadecimal);
				});
			}

			inline bool InputFraction(std::string_view label, ivec2& inOutValue, std::optional<ivec2> valueRange = {}, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputFractionBase(inOutValue, valueRange, disabledComponents); });
			}

			inline bool Input(std::string_view label, ivec2& inOutValue, f32 dragSpeed = 1.0f, std::optional<ivec2> dragRange = {}, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecNDragPropertyBase<ivec2, ivec2::value_type>(inOutValue, dragSpeed, dragRange, disabledComponents); });
			}

			inline bool Input(std::string_view label, ivec3& inOutValue, f32 dragSpeed = 1.0f, std::optional<ivec2> dragRange = {}, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecNDragPropertyBase<ivec3, ivec3::value_type>(inOutValue, dragSpeed, dragRange, disabledComponents); });
			}

			inline bool Input(std::string_view label, ivec4& inOutValue, f32 dragSpeed = 1.0f, std::optional<ivec2> dragRange = {}, ComponentFlags disabledComponents = ComponentFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { return Detail::InputVecNDragPropertyBase<ivec4, ivec4::value_type>(inOutValue, dragSpeed, dragRange, disabledComponents); });
			}

			inline bool Input(std::string_view label, char* inOutBuffer, size_t bufferSize, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { RAII::ItemWidth width(-1.0f); return ImGui::InputText(Detail::DummyLabel, inOutBuffer, bufferSize, flags); });
			}

			inline bool Input(std::string_view label, std::string& inOutValue, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { RAII::ItemWidth width(-1.0f); return ImGui::InputText(Detail::DummyLabel, &inOutValue, flags); });
			}

			inline bool InputWithHint(std::string_view label, const char* hint, std::string& inOutValue, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { RAII::ItemWidth width(-1.0f); return ImGui::InputTextWithHint(Detail::DummyLabel, hint, &inOutValue, flags); });
			}

			inline bool InputMultiline(std::string_view label, std::string& inOutValue, vec2 size = {}, ImGuiInputTextFlags flags = ImGuiInputTextFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { RAII::ItemWidth width(-1.0f); return ImGui::InputTextMultiline(Detail::DummyLabel, &inOutValue, size, flags); });
			}

			inline bool Checkbox(std::string_view label, bool& inOutValue)
			{
				RAII::ID id(label);
				return PropertyFuncValueFunc([&]
				{
					return Detail::TextButton(label, inOutValue);
				}, [&]
				{
					return ImGui::Checkbox(Detail::DummyLabel, &inOutValue);
				});
			}

			inline bool CheckboxFlags(std::string_view label, u32& inOutValue, u32 flagsMask)
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

			struct ComboResult
			{
				i32 HoveredIndex;
				bool IsOpen;
				bool ValueChanged;
			};

			// NOTE: IndexToStringFunc should handle index validation itself (range checking for arrays), nullptr returns will be skipped
			template <typename IndexToStringFunc>
			bool Combo(std::string_view label, i32& inOutIndex, i32 startRange, i32 endRange, ImGuiComboFlags flags, IndexToStringFunc indexToString, ComboResult* outResult = nullptr)
			{
				return PropertyLabelValueFunc(label, [&]
				{
					bool valueChanged = false;
					const char* previewValue = indexToString(inOutIndex);

					SetNextItemWidth(GetContentRegionAvail().x);
					if (ImGui::BeginCombo(Detail::DummyLabel, (previewValue == nullptr) ? "" : previewValue, flags))
					{
						if (outResult != nullptr)
						{
							outResult->IsOpen = true;
							outResult->HoveredIndex = -1;
						}

						for (i32 i = startRange; i < endRange; i++)
						{
							if (const char* itemLabel = indexToString(i); itemLabel != nullptr)
							{
								const bool isSelected = (i == inOutIndex);
								if (Selectable(itemLabel, isSelected))
								{
									inOutIndex = i;
									valueChanged = true;
								}

								if (outResult != nullptr && IsItemHovered())
									outResult->HoveredIndex = i;

								if (isSelected)
									SetItemDefaultFocus();
							}
						}
						EndCombo();
					}
					else if (outResult != nullptr)
						outResult->IsOpen = false;

					if (outResult != nullptr)
						outResult->ValueChanged = valueChanged;

					return valueChanged;
				});
			}

			template <typename IndexToStringFunc>
			bool Combo(std::string_view label, i32& inOutIndex, i32 startRange, i32 endRange, IndexToStringFunc indexToString)
			{
				return Combo(label, inOutIndex, startRange, endRange, ImGuiComboFlags_None, indexToString);
			}

			template <typename EnumType, size_t ArraySize>
			bool Combo(std::string_view label, EnumType& inOutEnum, const std::array<const char*, ArraySize>& nameLookup, ImGuiComboFlags flags = ImGuiComboFlags_None, i32 startRange = -1, i32 endRange = -1)
			{
				static_assert(sizeof(EnumType) <= sizeof(i32));

				i32 tempIndex = static_cast<i32>(inOutEnum);
				if (Combo(label, tempIndex, (startRange < 0) ? 0 : startRange, (endRange < 0) ? static_cast<i32>(ArraySize) : endRange, flags, [&](i32 index) { return Comfy::IndexOr(index, nameLookup, nullptr); }))
				{
					inOutEnum = static_cast<EnumType>(tempIndex);
					return true;
				}
				return false;
			}

			enum class Boolean : u8
			{
				False = false,
				True = true,
				Count
			};

			constexpr std::array<const char*, Comfy::EnumCount<Boolean>()> BooleanNames =
			{
				"False",
				"True",
			};

			// NOTE: Has the advantage over a checkbox of allowing no value to be set at all
			inline bool ComboBoolean(std::string_view label, Boolean& inOutValue, ImGuiComboFlags flags = ImGuiComboFlags_None)
			{
				return Combo(label, inOutValue, BooleanNames, flags);
			}

			inline bool ColorEdit(std::string_view label, vec3& inOutValue, ImGuiColorEditFlags flags = ImGuiColorEditFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { RAII::ItemWidth width(-1.0f); return ImGui::ColorEdit3(Detail::DummyLabel, glm::value_ptr(inOutValue), flags); });
			}

			inline bool ColorEdit(std::string_view label, vec4& inOutValue, ImGuiColorEditFlags flags = ImGuiColorEditFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { RAII::ItemWidth width(-1.0f); return ImGui::ColorEdit4(Detail::DummyLabel, glm::value_ptr(inOutValue), flags); });
			}

			inline bool ColorEditHDR(std::string_view label, vec3& inOutValue, ImGuiColorEditFlags flags = ImGuiColorEditFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { RAII::ItemWidth width(-1.0f); return ImGui::ColorEdit3(Detail::DummyLabel, glm::value_ptr(inOutValue), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | flags); });
			}

			inline bool ColorEditHDR(std::string_view label, vec4& inOutValue, ImGuiColorEditFlags flags = ImGuiColorEditFlags_None)
			{
				return PropertyLabelValueFunc(label, [&] { RAII::ItemWidth width(-1.0f); return ImGui::ColorEdit4(Detail::DummyLabel, glm::value_ptr(inOutValue), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float | flags); });
			}
		}
	}
}

namespace GuiProperty = ImGui::PropertyEditor::Widgets;
namespace GuiPropertyRAII = ImGui::PropertyEditor::RAII;

#ifndef COMFY_GUI_PROPERTY_NO_MACROS

#define GuiPropertyBitFieldCheckbox(name, bitFieldMember) \
{ 														  \
	bool temp = bitFieldMember; 						  \
	if (GuiProperty::Checkbox(name, temp)) 				  \
		bitFieldMember = temp; 							  \
}

#define GuiPropertyBitFieldInputInt(name, bitFieldMember) \
{ 														  \
	i32 temp = bitFieldMember; 							  \
	if (GuiProperty::Input(name, temp)) 				  \
		bitFieldMember = temp;							  \
}

#define GuiPropertyBitFieldComboEnum(name, bitfieldMember, enumNames) \
{ 																	  \
	i32 temp = static_cast<i32>(bitfieldMember); 					  \
	if (GuiProperty::Combo(name, temp, enumNames)) 					  \
		bitfieldMember = static_cast<decltype(bitfieldMember)>(temp); \
}

#endif // !COMFY_GUI_PROPERTY_NO_MACROS
