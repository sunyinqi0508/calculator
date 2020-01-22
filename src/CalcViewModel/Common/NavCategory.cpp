// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#include "pch.h"
#include "NavCategory.h"
#include "AppResourceProvider.h"
#include "Common/LocalizationStringUtil.h"
#include <initializer_list>

using namespace CalculatorApp;
using namespace CalculatorApp::Common;
using namespace Platform;
using namespace Platform::Collections;
using namespace std;
using namespace Windows::Foundation::Collections;

namespace UCM = UnitConversionManager;

// Calculator categories always support negative and positive.
static constexpr bool SUPPORTS_ALL = true;

// Converter categories usually only support positive.
static constexpr bool SUPPORTS_NEGATIVE = true;
static constexpr bool POSITIVE_ONLY = false;

// vvv THESE CONSTANTS SHOULD NEVER CHANGE vvv
static constexpr int STANDARD_ID = 0;
static constexpr int SCIENTIFIC_ID = 1;
static constexpr int PROGRAMMER_ID = 2;
static constexpr int DATE_ID = 3;
static constexpr int VOLUME_ID = 4;
static constexpr int LENGTH_ID = 5;
static constexpr int WEIGHT_ID = 6;
static constexpr int TEMPERATURE_ID = 7;
static constexpr int ENERGY_ID = 8;
static constexpr int AREA_ID = 9;
static constexpr int SPEED_ID = 10;
static constexpr int TIME_ID = 11;
static constexpr int POWER_ID = 12;
static constexpr int DATA_ID = 13;
static constexpr int PRESSURE_ID = 14;
static constexpr int ANGLE_ID = 15;
static constexpr int CURRENCY_ID = 16;
static constexpr int GRAPHING_ID = 17;
// ^^^ THESE CONSTANTS SHOULD NEVER CHANGE ^^^

wchar_t* towchar_t(int number)
{
    auto wstr = to_wstring(number);
    return _wcsdup(wstr.c_str());
}

extern "C"
{
    WINADVAPI LSTATUS APIENTRY RegGetValueW(
        _In_ HKEY hkey,
        _In_opt_ LPCWSTR lpSubKey,
        _In_opt_ LPCWSTR lpValue,
        _In_ DWORD dwFlags,
        _Out_opt_ LPDWORD pdwType,
        _When_(
            (dwFlags & 0x7F) == RRF_RT_REG_SZ || (dwFlags & 0x7F) == RRF_RT_REG_EXPAND_SZ || (dwFlags & 0x7F) == (RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ)
                || *pdwType == REG_SZ || *pdwType == REG_EXPAND_SZ,
            _Post_z_) _When_((dwFlags & 0x7F) == RRF_RT_REG_MULTI_SZ || *pdwType == REG_MULTI_SZ, _Post_ _NullNull_terminated_)
            _Out_writes_bytes_to_opt_(*pcbData, *pcbData) PVOID pvData,
        _Inout_opt_ LPDWORD pcbData);
}

bool IsGraphingModeAvailable()
{
    static bool supportGraph = Windows::Foundation::Metadata::ApiInformation::IsMethodPresent("Windows.UI.Text.RichEditTextDocument", "GetMath");
    return supportGraph;
}

Box<bool> ^ _isGraphingModeEnabledCached = nullptr;
bool IsGraphingModeEnabled()
{
    if (!IsGraphingModeAvailable())
    {
        return false;
    }

    if (_isGraphingModeEnabledCached != nullptr)
    {
        return _isGraphingModeEnabledCached->Value;
    }

    DWORD allowGraphingCalculator{ 0 };
    DWORD bufferSize{ sizeof(allowGraphingCalculator) };
    // Make sure to call RegGetValueW only on Windows 10 1903+
    if (RegGetValueW(
            HKEY_CURRENT_USER,
            L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\Calculator",
            L"AllowGraphingCalculator",
            RRF_RT_DWORD, // RRF_RT_DWORD == RRF_RT_REG_DWORD | RRF_RT_REG_BINARY
            nullptr,
            reinterpret_cast<LPBYTE>(&allowGraphingCalculator),
            &bufferSize)
        == ERROR_SUCCESS)
    {
        _isGraphingModeEnabledCached = allowGraphingCalculator != 0;
    }
    else
    {
        _isGraphingModeEnabledCached = true;
    }
    return _isGraphingModeEnabledCached->Value;
}

// The order of items in this list determines the order of items in the menu.
static const list<NavCategoryInitializer> s_categoryManifest = [] {
    auto res = list<NavCategoryInitializer>{ NavCategoryInitializer{ ViewMode::Standard,
                                                                     STANDARD_ID,
                                                                     L"Standard",
                                                                     L"StandardMode",
                                                                     L"\uE8EF",
                                                                     CategoryGroupType::Calculator,
                                                                     MyVirtualKey::Number1,
                                                                     L"1",
                                                                     SUPPORTS_ALL,
                                                                     true,
                                                                     false },
                                             NavCategoryInitializer{ ViewMode::Scientific,
                                                                     SCIENTIFIC_ID,
                                                                     L"Scientific",
                                                                     L"ScientificMode",
                                                                     L"\uF196",
                                                                     CategoryGroupType::Calculator,
                                                                     MyVirtualKey::Number2,
                                                                     L"2",
                                                                     SUPPORTS_ALL,
                                                                     true,
                                                                     false } };

    int currentIndex = 3;
    bool supportGraphingCalculator = IsGraphingModeAvailable();
    if (supportGraphingCalculator)
    {
        const bool isEnabled = IsGraphingModeEnabled();
        res.push_back(NavCategoryInitializer{ ViewMode::Graphing,
                                              GRAPHING_ID,
                                              L"Graphing",
                                              L"GraphingCalculatorMode",
                                              L"\uF770",
                                              CategoryGroupType::Calculator,
                                              MyVirtualKey::Number3,
                                              L"3",
                                              SUPPORTS_ALL,
                                              isEnabled,
                                              true });
        ++currentIndex;
    }
    res.insert(
        res.end(),
        { NavCategoryInitializer{ ViewMode::Programmer,
                                  PROGRAMMER_ID,
                                  L"Programmer",
                                  L"ProgrammerMode",
                                  L"\uECCE",
                                  CategoryGroupType::Calculator,
                                  supportGraphingCalculator ? MyVirtualKey::Number4 : MyVirtualKey::Number3,
                                  towchar_t(currentIndex++),
                                  SUPPORTS_ALL,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Date,
                                  DATE_ID,
                                  L"Date",
                                  L"DateCalculationMode",
                                  L"\uE787",
                                  CategoryGroupType::Calculator,
                                  supportGraphingCalculator ? MyVirtualKey::Number5 : MyVirtualKey::Number4,
                                  towchar_t(currentIndex++),
                                  SUPPORTS_ALL,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Currency,
                                  CURRENCY_ID,
                                  L"Currency",
                                  L"CategoryName_Currency",
                                  L"\uEB0D",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Volume,
                                  VOLUME_ID,
                                  L"Volume",
                                  L"CategoryName_Volume",
                                  L"\uF1AA",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Length,
                                  LENGTH_ID,
                                  L"Length",
                                  L"CategoryName_Length",
                                  L"\uECC6",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Weight,
                                  WEIGHT_ID,
                                  L"Weight and Mass",
                                  L"CategoryName_Weight",
                                  L"\uF4C1",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Temperature,
                                  TEMPERATURE_ID,
                                  L"Temperature",
                                  L"CategoryName_Temperature",
                                  L"\uE7A3",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  SUPPORTS_NEGATIVE,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Energy,
                                  ENERGY_ID,
                                  L"Energy",
                                  L"CategoryName_Energy",
                                  L"\uECAD",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Area,
                                  AREA_ID,
                                  L"Area",
                                  L"CategoryName_Area",
                                  L"\uE809",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Speed,
                                  SPEED_ID,
                                  L"Speed",
                                  L"CategoryName_Speed",
                                  L"\uEADA",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Time,
                                  TIME_ID,
                                  L"Time",
                                  L"CategoryName_Time",
                                  L"\uE917",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Power,
                                  POWER_ID,
                                  L"Power",
                                  L"CategoryName_Power",
                                  L"\uE945",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  SUPPORTS_NEGATIVE,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Data,
                                  DATA_ID,
                                  L"Data",
                                  L"CategoryName_Data",
                                  L"\uF20F",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Pressure,
                                  PRESSURE_ID,
                                  L"Pressure",
                                  L"CategoryName_Pressure",
                                  L"\uEC4A",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  POSITIVE_ONLY,
                                  true,
                                  false },
          NavCategoryInitializer{ ViewMode::Angle,
                                  ANGLE_ID,
                                  L"Angle",
                                  L"CategoryName_Angle",
                                  L"\uF515",
                                  CategoryGroupType::Converter,
                                  MyVirtualKey::None,
                                  nullptr,
                                  SUPPORTS_NEGATIVE,
                                  true,
                                  false } });
    return res;
}();

// This function should only be used when storing the mode to app data.
int NavCategory::Serialize(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode; });

    return (iter != s_categoryManifest.end()) ? iter->serializationId : -1;
}

// This function should only be used when restoring the mode from app data.
ViewMode NavCategory::Deserialize(Platform::Object ^ obj)
{
    // If we cast directly to ViewMode we will fail
    // because we technically store an int.
    // Need to cast to int, then ViewMode.
    auto boxed = dynamic_cast<Box<int> ^>(obj);
    if (boxed != nullptr)
    {
        int serializationId = boxed->Value;
        auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [serializationId](const NavCategoryInitializer& initializer) {
            return initializer.serializationId == serializationId;
        });

        if (iter != s_categoryManifest.end())
        {
            if (iter->viewMode == ViewMode::Graphing)
            {
                // check if the user is allowed to use this feature
                if (!IsGraphingModeEnabled())
                {
                    return ViewMode::None;
                }
            }
            return iter->viewMode;
        }
    }

    return ViewMode::None;
}

bool NavCategory::IsValidViewMode(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode; });

    return iter != s_categoryManifest.end();
}

bool NavCategory::IsCalculatorViewMode(ViewMode mode)
{
    // Historically, Calculator modes are Standard, Scientific, and Programmer.
    return !IsDateCalculatorViewMode(mode) && !IsGraphingCalculatorViewMode(mode) && IsModeInCategoryGroup(mode, CategoryGroupType::Calculator);
}

bool NavCategory::IsGraphingCalculatorViewMode(ViewMode mode)
{
    return mode == ViewMode::Graphing;
}

bool NavCategory::IsDateCalculatorViewMode(ViewMode mode)
{
    return mode == ViewMode::Date;
}

bool NavCategory::IsConverterViewMode(ViewMode mode)
{
    return IsModeInCategoryGroup(mode, CategoryGroupType::Converter);
}

bool NavCategory::IsModeInCategoryGroup(ViewMode mode, CategoryGroupType type)
{
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode, type](const NavCategoryInitializer& initializer) {
        return initializer.viewMode == mode && initializer.groupType == type;
    });

    return iter != s_categoryManifest.end();
}

String ^ NavCategory::GetFriendlyName(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode; });

    return (iter != s_categoryManifest.end()) ? StringReference(iter->friendlyName) : L"None";
}

ViewMode NavCategory::GetViewModeForFriendlyName(String ^ name)
{
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [name](const NavCategoryInitializer& initializer) {
        return wcscmp(initializer.friendlyName, name->Data()) == 0;
    });

    return (iter != s_categoryManifest.end()) ? iter->viewMode : ViewMode::None;
}

String ^ NavCategory::GetNameResourceKey(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode; });

    return (iter != s_categoryManifest.end()) ? StringReference(iter->nameResourceKey) + "Text" : nullptr;
}

CategoryGroupType NavCategory::GetGroupType(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode; });

    return (iter != s_categoryManifest.end()) ? iter->groupType : CategoryGroupType::None;
}

// GetIndex is 0-based, GetPosition is 1-based
int NavCategory::GetIndex(ViewMode mode)
{
    int position = NavCategory::GetPosition(mode);
    return max(-1, position - 1);
}

int NavCategory::GetFlatIndex(ViewMode mode)
{
    int index = -1;
    CategoryGroupType type = CategoryGroupType::None;
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode, &type, &index](const NavCategoryInitializer& initializer) {
        index++;
        if (initializer.groupType != type)
        {
            type = initializer.groupType;
            index++;
        }

        return initializer.viewMode == mode;
    });

    return (iter != s_categoryManifest.end()) ? index : -1;
}

// GetIndex is 0-based, GetPosition is 1-based
int NavCategory::GetIndexInGroup(ViewMode mode, CategoryGroupType type)
{
    int index = -1;
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode, type, &index](const NavCategoryInitializer& initializer) {
        if (initializer.groupType == type)
        {
            index++;
            return initializer.viewMode == mode;
        }

        return false;
    });

    return (iter != s_categoryManifest.end()) ? index : -1;
}

// GetIndex is 0-based, GetPosition is 1-based
int NavCategory::GetPosition(ViewMode mode)
{
    int position = 0;
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode, &position](const NavCategoryInitializer& initializer) {
        position++;
        return initializer.viewMode == mode;
    });

    return (iter != s_categoryManifest.end()) ? position : -1;
}

ViewMode NavCategory::GetViewModeForVirtualKey(MyVirtualKey virtualKey)
{
    auto iter = find_if(begin(s_categoryManifest), end(s_categoryManifest), [virtualKey](const NavCategoryInitializer& initializer) {
        return initializer.virtualKey == virtualKey;
    });

    return (iter != s_categoryManifest.end()) ? iter->viewMode : ViewMode::None;
}

vector<MyVirtualKey> NavCategory::GetCategoryAcceleratorKeys()
{
    vector<MyVirtualKey> accelerators{};
    for (auto category : s_categoryManifest)
    {
        if (category.virtualKey != MyVirtualKey::None)
        {
            accelerators.push_back(category.virtualKey);
        }
    }

    return accelerators;
}

NavCategoryGroup::NavCategoryGroup(const NavCategoryGroupInitializer& groupInitializer)
    : m_Categories(ref new Vector<NavCategory ^>())
{
    m_GroupType = groupInitializer.type;

    auto resProvider = AppResourceProvider::GetInstance();
    m_Name = resProvider->GetResourceString(StringReference(groupInitializer.headerResourceKey));
    String ^ groupMode = resProvider->GetResourceString(StringReference(groupInitializer.modeResourceKey));
    String ^ automationName = resProvider->GetResourceString(StringReference(groupInitializer.automationResourceKey));

    String ^ navCategoryHeaderAutomationNameFormat = resProvider->GetResourceString(L"NavCategoryHeader_AutomationNameFormat");
    m_AutomationName = LocalizationStringUtil::GetLocalizedString(navCategoryHeaderAutomationNameFormat, automationName);

    String ^ navCategoryItemAutomationNameFormat = resProvider->GetResourceString(L"NavCategoryItem_AutomationNameFormat");

    for (const NavCategoryInitializer& categoryInitializer : s_categoryManifest)
    {
        if (categoryInitializer.groupType == groupInitializer.type)
        {
            String ^ nameResourceKey = StringReference(categoryInitializer.nameResourceKey);
            String ^ categoryName = resProvider->GetResourceString(nameResourceKey + "Text");
            String ^ categoryAutomationName = LocalizationStringUtil::GetLocalizedString(navCategoryItemAutomationNameFormat, categoryName, m_Name);

            m_Categories->Append(ref new NavCategory(
                categoryName,
                categoryAutomationName,
                StringReference(categoryInitializer.glyph),
                categoryInitializer.accessKey != nullptr ? ref new String(categoryInitializer.accessKey)
                                                         : resProvider->GetResourceString(nameResourceKey + "AccessKey"),
                groupMode,
                categoryInitializer.viewMode,
                categoryInitializer.supportsNegative,
                categoryInitializer.isEnabled,
                categoryInitializer.isPreview));
        }
    }
}

IObservableVector<NavCategoryGroup ^> ^ NavCategoryGroup::CreateMenuOptions()
{
    auto menuOptions = ref new Vector<NavCategoryGroup ^>();
    menuOptions->Append(CreateCalculatorCategory());
    menuOptions->Append(CreateConverterCategory());
    return menuOptions;
}

NavCategoryGroup ^ NavCategoryGroup::CreateCalculatorCategory()
{
    return ref new NavCategoryGroup(
        NavCategoryGroupInitializer{ CategoryGroupType::Calculator, L"CalculatorModeTextCaps", L"CalculatorModeText", L"CalculatorModePluralText" });
}

NavCategoryGroup ^ NavCategoryGroup::CreateConverterCategory()
{
    return ref new NavCategoryGroup(
        NavCategoryGroupInitializer{ CategoryGroupType::Converter, L"ConverterModeTextCaps", L"ConverterModeText", L"ConverterModePluralText" });
}

bool NavCategory::IsViewModePreview(ViewMode mode)
{
    auto iter =
        find_if(begin(s_categoryManifest), end(s_categoryManifest), [mode](const NavCategoryInitializer& initializer) { return initializer.viewMode == mode; });

    if (iter != s_categoryManifest.end())
    {
        return iter->isPreview;
    }
    return false;
}
