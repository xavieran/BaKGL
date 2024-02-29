#include "bak/dialogReplacements.hpp"

namespace BAK {

const std::vector<Replacement> Replacements::replacements = {
    Replacement{OffsetTarget{31, 0xb0f}, {
        {0, PlaySound{34, 0, {}}}
    }},
    Replacement{OffsetTarget{31, 0xe35}, {
        {0, PlaySound{72, 0, {}}},
        {1, PlaySound{72, 0, {}}},
        {2, PlaySound{72, 0, {}}}
    }},
    Replacement{OffsetTarget{31, 0xf5a}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{38, 0, {}}},
        {2, PlaySound{60, 2, {}}},
        {3, PlaySound{40, 2, {}}},
        {4, PlaySound{53, 2, {}}}
    }},
    Replacement{OffsetTarget{31, 0x1196}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{34, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x11f3}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x139c}, {
        {0, PlaySound{5, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1cfc}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1db8}, {
        {0, PlaySound{39, 2, {}}},
    }},
    Replacement{OffsetTarget{31, 0x21da}, {
        {0, PlaySound{38, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x30ff}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x336d}, {
        {0, PlaySound{38, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x34bf}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x3564}, {
        {0, PlaySound{34, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x3816}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x3a12}, {
        {0, PlaySound{38, 2, {}}},
    }},
    Replacement{OffsetTarget{31, 0x4054}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x445f}, {
        {0, PlaySound{38, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x4830}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x5170}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x52a8}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{34, 0, {}}},
        {2, PlaySound{34, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x5479}, {
        {0, PlaySound{75, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x5503}, {
        {0, PlaySound{67, 0, {}}},
        {1, PlaySound{67, 0, {}}},
        {2, PlaySound{67, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x57db}, {
        {0, PlaySound{38, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x61ae}, {
        {0, PlaySound{38, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x66d5}, {
        {0, PlaySound{72, 0, {}}},
        {1, PlaySound{72, 0, {}}},
        {2, PlaySound{72, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x694e}, {
        {0, PlaySound{38, 2, {}}},
    }},
    Replacement{OffsetTarget{31, 0x6e2e}, {
        {0, PlaySound{67, 0, {}}},
        {1, PlaySound{67, 0, {}}},
        {2, PlaySound{67, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x6f08}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{38, 2, {}}},
    }},
    Replacement{OffsetTarget{31, 0x7293}, {
        {0, PlaySound{38, 1, {}}},
        {1, PlaySound{39, 2, {}}},
    }},
    Replacement{OffsetTarget{31, 0x75a2}, {
        {0, PlaySound{39, 1, {}}},
        {1, PlaySound{66, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x77fe}, {
        {0, PlaySound{38, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x7fc1}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{34, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x892f}, {
        {0, PlaySound{38, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x8ca8}, {
        {0, PlaySound{38, 2, {}}},
        {1, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x8eb8}, {
        {0, PlaySound{67, 0, {}}},
        {1, PlaySound{67, 0, {}}},
        {2, PlaySound{67, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x91b5}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x95dc}, {
        {0, PlaySound{67, 0, {}}},
        {1, PlaySound{38, 1, {}}},
        {2, PlaySound{40, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x9d2f}, {
        {0, SetTextVariable{0, 13, {}}},
        {1, GainSkill{2, SkillType::TotalHealth, -4120, -2560}},
        {2, PlaySound{26, 1, {}}},
        {3, PlaySound{40, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0xa134}, {
        {0, LoseItem{53, 25, {}}},
        {1, PlaySound{64, 0, {}}},
        {2, PlaySound{64, 0, {}}},
        {3, PlaySound{63, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0xab5c}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0xb7af}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0xc35e}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{39, 2, {}}},
    }},
    Replacement{OffsetTarget{31, 0xc5e9}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{34, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0xc6b8}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0xcee7}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0xdad3}, {
        {0, PlaySound{67, 0, {}}},
        {1, PlaySound{67, 0, {}}},
        {2, PlaySound{67, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0xdeae}, {
        {0, PlaySound{67, 0, {}}},
        {1, PlaySound{67, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0xe182}, {
        {0, PlaySound{38, 0, {}}},
    }},
    // This is presumably intentional
    //Replacement{OffsetTarget{31, 0xeab2}, {
    //    {0, GiveItem{2, 5, 50, {}}},
    //}},
    Replacement{OffsetTarget{31, 0xf312}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{72, 0, {}}},
        {2, PlaySound{72, 0, {}}},
        {3, PlaySound{72, 0, {}}},
        {4, PlaySound{39, 2, {}}},
    }},
    Replacement{OffsetTarget{31, 0xf7f2}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{66, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0xfa52}, {
        {0, PlaySound{34, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0xfdbe}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x10226}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x10563}, {
        {0, PlaySound{67, 0, {}}},
        {1, PlaySound{67, 0, {}}},
        {2, PlaySound{67, 0, {}}},
        {3, PlaySound{67, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x112da}, {
        {0, PlaySound{64, 0, {}}},
        {1, PlaySound{64, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x11740}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x12015}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{38, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x12404}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{38, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x126fb}, {
        {0, SetFlag{0x1f03, 0, 0, 0, 1}},
        {1, PlaySound{34, 0, {}}},
        {2, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x12921}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x12a7b}, {
        {0, PlaySound{38, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x12ea3}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x12fb2}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x131c6}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x140af}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x143ef}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{38, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1491e}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x14c58}, {
        {0, PlaySound{67, 0, {}}},
        {1, PlaySound{67, 0, {}}},
        {2, PlaySound{67, 0, {}}},
        {3, PlaySound{67, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x155af}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x15c9e}, {
        {0, ElapseTime{Times::ThreeHours, {}}},
        {1, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x16978}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x170ee}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x17bf9}, {
        {1, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x17e15}, {
        {0, PlaySound{72, 1, {}}},
        {1, PlaySound{72, 1, {}}},
        {2, PlaySound{72, 1, {}}},
        {3, PlaySound{72, 1, {}}},
        {4, PlaySound{20, 2, {}}},
    }},
    Replacement{OffsetTarget{31, 0x17fd1}, {
        {0, PlaySound{72, 1, {}}},
        {1, PlaySound{72, 1, {}}},
        {2, PlaySound{72, 1, {}}},
        {3, PlaySound{72, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x18a4e}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x19328}, {
        {0, PlaySound{39, 2, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1953d}, {
        {0, PlaySound{1007, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1955a}, {
        {0, PlaySound{1039, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x19577}, {
        {0, PlaySound{1040, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1993f}, {
        {0, SetFlag{0xdbba, 0xff, 0x40, 0, 0}},
        {1, PlaySound{1008, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x19e38}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1a438}, {
        {0, PlaySound{64, 0, {}}},
        {1, PlaySound{64, 0, {}}},
        {2, PlaySound{63, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1a7fb}, {
        {0, PlaySound{64, 0, {}}},
        {1, PlaySound{64, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1af8f}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{38, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1b2df}, {
        {0, PlaySound{38, 2, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1b3b9}, {
        {0, PlaySound{34, 1, {}}},
        {1, PlaySound{38, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1b787}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{72, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1bb40}, {
        {0, PlaySound{21, 1, {}}},
        {1, PlaySound{21, 1, {}}},
        {2, PlaySound{21, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1d34f}, {
        {0, PlaySound{64, 0, {}}},
        {1, PlaySound{64, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1d6a9}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1ea01}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{38, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1eed0}, {
        {0, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1fd64}, {
        {0, PlaySound{72, 0, {}}},
        {1, PlaySound{72, 0, {}}},
        {2, PlaySound{72, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x1fef5}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{34, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x2055a}, {
        {0, PlaySound{64, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x21269}, {
        {0, PlaySound{29, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x219f9}, {
        {0, PlaySound{39, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x219f9}, {
        {0, PlaySound{39, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x21ef6}, {
        {0, PlaySound{34, 0, {}}},
        {1, PlaySound{67, 0, {}}},
        {2, PlaySound{67, 0, {}}},
    }},
    Replacement{OffsetTarget{31, 0x221e2}, {
        {0, PlaySound{38, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x22c2a}, {
        {0, SetEndOfDialogState{-1, {}}},
        {2, PlaySound{29, 2, {}}},
    }},
    // Fadamor's formula well...
    Replacement{OffsetTarget{31, 0x23986}, {
        {0, LoseItem{115, 1, {}}},
        {2, PlaySound{63, 2, {}}},
    }},
    Replacement{OffsetTarget{31, 0x23da6}, {
        {0, PlaySound{67, 0, {}}},
        {1, PlaySound{67, 0, {}}},
        {2, PlaySound{34, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x24332}, {
        {0, SetFlag{0x1f36, 0, 0, 0, 1}},
        {1, GainSkill{7, SkillType::TotalHealth, -5120, -5120}},
        {2, PlaySound{66, 1, {}}},
        {3, PlaySound{40, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x26a01}, {
        {4, LoseItem{62, 1, {}}},
        {5, LoseItem{53, 100, {}}},
        {6, PlaySound{66, 1, {}}},
        {7, PlaySound{12, 1, {}}},
    }},
    Replacement{OffsetTarget{31, 0x28008}, {
        {0, PlaySound{75, 1, {}}},
    }},
};

}
