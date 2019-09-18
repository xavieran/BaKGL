/*
 * This file is part of xBaK.
 *
 * xBaK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * xBaK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with xBaK.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) Guido de Jong <guidoj@users.sf.net>
 */

#include "SkillsWidget.h"

SkillsWidget::SkillsWidget(const Rectangle &r, PlayerCharacter *pc, Image *sw, Image *bl, Font *f)
        : Widget(r)
        , playerCharacter(pc)
        , font(f)
        , leftSword(0)
        , leftBlood(0)
        , rightSword(0)
        , rightBlood(0)
        , defense(0)
        , crossbowAccuracy(0)
        , meleeAccuracy(0)
        , castingAccuracy(0)
        , assessment(0)
        , armorcraft(0)
        , weaponcraft(0)
        , barding(0)
        , haggling(0)
        , lockpick(0)
        , scouting(0)
        , stealth(0)
{
    leftSword = new Image(sw);
    leftBlood = new Image(bl);
    rightSword = new Image(sw);
    rightBlood = new Image(bl);
    rightSword->HorizontalFlip();
    rightBlood->HorizontalFlip();
    defense = new SkillLevelWidget(Rectangle(rect.GetXPos(), rect.GetYPos(), 132, 25), LEFT_SIDE, leftSword, leftBlood, f);
    crossbowAccuracy = new SkillLevelWidget(Rectangle(rect.GetXPos(), rect.GetYPos() + 16, 132, 25), LEFT_SIDE, leftSword, leftBlood, f);
    meleeAccuracy = new SkillLevelWidget(Rectangle(rect.GetXPos(), rect.GetYPos() + 32, 132, 25), LEFT_SIDE, leftSword, leftBlood, f);
    castingAccuracy = new SkillLevelWidget(Rectangle(rect.GetXPos(), rect.GetYPos() + 48, 132, 25), LEFT_SIDE, leftSword, leftBlood, f);
    assessment = new SkillLevelWidget(Rectangle(rect.GetXPos(), rect.GetYPos() + 64, 132, 25), LEFT_SIDE, leftSword, leftBlood, f);
    armorcraft = new SkillLevelWidget(Rectangle(rect.GetXPos(), rect.GetYPos() + 80, 132, 25), LEFT_SIDE, leftSword, leftBlood, f);
    weaponcraft = new SkillLevelWidget(Rectangle(rect.GetXPos() + 145, rect.GetYPos(), 132, 25), RIGHT_SIDE, rightSword, rightBlood, f);
    barding = new SkillLevelWidget(Rectangle(rect.GetXPos() + 145, rect.GetYPos() + 16, 132, 25), RIGHT_SIDE, rightSword, rightBlood, f);
    haggling = new SkillLevelWidget(Rectangle(rect.GetXPos() + 145, rect.GetYPos() + 32, 132, 25), RIGHT_SIDE, rightSword, rightBlood, f);
    lockpick = new SkillLevelWidget(Rectangle(rect.GetXPos() + 145, rect.GetYPos() + 48, 132, 25), RIGHT_SIDE, rightSword, rightBlood, f);
    scouting = new SkillLevelWidget(Rectangle(rect.GetXPos() + 145, rect.GetYPos() + 64, 132, 25), RIGHT_SIDE, rightSword, rightBlood, f);
    stealth = new SkillLevelWidget(Rectangle(rect.GetXPos() + 145, rect.GetYPos() + 80, 132, 25), RIGHT_SIDE, rightSword, rightBlood, f);
    defense->SetLevel("Defense", pc->GetStatistics().Get(STAT_DEFENSE, STAT_ACTUAL));
    crossbowAccuracy->SetLevel("Accy: Crossbow", pc->GetStatistics().Get(STAT_CROSSBOW_ACCURACY, STAT_ACTUAL));
    meleeAccuracy->SetLevel("Accy: Melee", pc->GetStatistics().Get(STAT_MELEE_ACCURACY, STAT_ACTUAL));
    castingAccuracy->SetLevel("Accy: Casting", pc->GetStatistics().Get(STAT_CASTING_ACCURACY, STAT_ACTUAL));
    assessment->SetLevel("Assessment", pc->GetStatistics().Get(STAT_ASSESSMENT, STAT_ACTUAL));
    armorcraft->SetLevel("Armorcraft", pc->GetStatistics().Get(STAT_ARMORCRAFT, STAT_ACTUAL));
    weaponcraft->SetLevel("Weaponcraft", pc->GetStatistics().Get(STAT_WEAPONCRAFT, STAT_ACTUAL));
    barding->SetLevel("Barding", pc->GetStatistics().Get(STAT_BARDING, STAT_ACTUAL));
    haggling->SetLevel("Haggling", pc->GetStatistics().Get(STAT_HAGGLING, STAT_ACTUAL));
    lockpick->SetLevel("Lockpick", pc->GetStatistics().Get(STAT_LOCKPICK, STAT_ACTUAL));
    scouting->SetLevel("Scouting", pc->GetStatistics().Get(STAT_SCOUTING, STAT_ACTUAL));
    stealth->SetLevel("Stealth", pc->GetStatistics().Get(STAT_STEALTH, STAT_ACTUAL));
}

SkillsWidget::~SkillsWidget()
{
    delete defense;
    delete crossbowAccuracy;
    delete meleeAccuracy;
    delete castingAccuracy;
    delete assessment;
    delete armorcraft;
    delete weaponcraft;
    delete barding;
    delete haggling;
    delete lockpick;
    delete scouting;
    delete stealth;
    delete leftSword;
    delete leftBlood;
    delete rightSword;
    delete rightBlood;
}

void
SkillsWidget::Draw()
{
    if (IsVisible())
    {
        defense->Draw();
        crossbowAccuracy->Draw();
        meleeAccuracy->Draw();
        castingAccuracy->Draw();
        assessment->Draw();
        armorcraft->Draw();
        weaponcraft->Draw();
        barding->Draw();
        haggling->Draw();
        lockpick->Draw();
        scouting->Draw();
        stealth->Draw();
    }
}

void
SkillsWidget::Drag(const int, const int)
{}

void
SkillsWidget::Drop(const int, const int)
{}
