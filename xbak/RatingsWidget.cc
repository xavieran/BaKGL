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

#include <iomanip>
#include <sstream>

#include "Exception.h"
#include "MediaToolkit.h"
#include "RatingsWidget.h"

RatingsWidget::RatingsWidget(const Rectangle &r, PlayerCharacter *pc, Font *f)
        : Widget(r)
        , playerCharacter(pc)
        , horizontalBorder(0)
        , verticalBorder(0)
        , ratingsLabel(0)
        , conditionLabel(0)
        , healthLabel(0)
        , staminaLabel(0)
        , speedLabel(0)
        , strengthLabel(0)
        , healthOfLabel(0)
        , staminaOfLabel(0)
        , actualHealth(0)
        , actualStamina(0)
        , actualSpeed(0)
        , actualStrength(0)
        , maximumHealth(0)
        , maximumStamina(0)
        , condition(0)
{
    ratingsLabel = new TextWidget(Rectangle(rect.GetXPos() + 11, rect.GetYPos() + 5, 40, 11), f);
    ratingsLabel->SetText("Ratings:");
    ratingsLabel->SetAlignment(HA_LEFT, VA_TOP);
    ratingsLabel->SetColor(INFO_TEXT_COLOR);
    ratingsLabel->SetShadow(COLOR_BLACK, 1, 1);
    conditionLabel = new TextWidget(Rectangle(rect.GetXPos() + 126, rect.GetYPos() + 5, 50, 11), f);
    conditionLabel->SetText("Condition:");
    conditionLabel->SetAlignment(HA_LEFT, VA_TOP);
    conditionLabel->SetColor(INFO_TEXT_COLOR);
    conditionLabel->SetShadow(COLOR_BLACK, 1, 1);
    healthLabel = new TextWidget(Rectangle(rect.GetXPos() + 21, rect.GetYPos() + 19, 40, 11), f);
    healthLabel->SetText("Health");
    healthLabel->SetAlignment(HA_LEFT, VA_TOP);
    healthLabel->SetColor(INFO_TEXT_COLOR);
    healthLabel->SetShadow(COLOR_BLACK, 1, 1);
    staminaLabel = new TextWidget(Rectangle(rect.GetXPos() + 21, rect.GetYPos() + 30, 40, 11), f);
    staminaLabel->SetText("Stamina");
    staminaLabel->SetAlignment(HA_LEFT, VA_TOP);
    staminaLabel->SetColor(INFO_TEXT_COLOR);
    staminaLabel->SetShadow(COLOR_BLACK, 1, 1);
    speedLabel = new TextWidget(Rectangle(rect.GetXPos() + 21, rect.GetYPos() + 41, 40, 11), f);
    speedLabel->SetText("Speed");
    speedLabel->SetAlignment(HA_LEFT, VA_TOP);
    speedLabel->SetColor(INFO_TEXT_COLOR);
    speedLabel->SetShadow(COLOR_BLACK, 1, 1);
    strengthLabel = new TextWidget(Rectangle(rect.GetXPos() + 21, rect.GetYPos() + 52, 40, 11), f);
    strengthLabel->SetText("Strength");
    strengthLabel->SetAlignment(HA_LEFT, VA_TOP);
    strengthLabel->SetColor(INFO_TEXT_COLOR);
    strengthLabel->SetShadow(COLOR_BLACK, 1, 1);
    healthOfLabel = new TextWidget(Rectangle(rect.GetXPos() + 86, rect.GetYPos() + 19, 12, 11), f);
    healthOfLabel->SetText("of");
    healthOfLabel->SetAlignment(HA_LEFT, VA_TOP);
    healthOfLabel->SetColor(INFO_TEXT_COLOR);
    healthOfLabel->SetShadow(COLOR_BLACK, 1, 1);
    staminaOfLabel = new TextWidget(Rectangle(rect.GetXPos() + 86, rect.GetYPos() + 30, 12, 11), f);
    staminaOfLabel->SetText("of");
    staminaOfLabel->SetAlignment(HA_LEFT, VA_TOP);
    staminaOfLabel->SetColor(INFO_TEXT_COLOR);
    staminaOfLabel->SetShadow(COLOR_BLACK, 1, 1);

    std::stringstream actualHealthStream;
    actualHealthStream << std::setw(2) << std::setfill(' ') << playerCharacter->GetStatistics().Get(STAT_HEALTH, STAT_ACTUAL);
    actualHealth = new TextWidget(Rectangle(rect.GetXPos() + 68, rect.GetYPos() + 19, 12, 11), f);
    actualHealth->SetText(actualHealthStream.str());
    actualHealth->SetAlignment(HA_RIGHT, VA_TOP);
    actualHealth->SetColor(INFO_TEXT_COLOR);
    actualHealth->SetShadow(COLOR_BLACK, 1, 1);
    std::stringstream actualStaminaStream;
    actualStaminaStream << std::setw(2) << std::setfill(' ') << playerCharacter->GetStatistics().Get(STAT_STAMINA, STAT_ACTUAL);
    actualStamina = new TextWidget(Rectangle(rect.GetXPos() + 68, rect.GetYPos() + 30, 12, 11), f);
    actualStamina->SetText(actualStaminaStream.str());
    actualStamina->SetAlignment(HA_RIGHT, VA_TOP);
    actualStamina->SetColor(INFO_TEXT_COLOR);
    actualStamina->SetShadow(COLOR_BLACK, 1, 1);
    std::stringstream actualSpeedStream;
    actualSpeedStream << std::setw(2) << std::setfill(' ') << playerCharacter->GetStatistics().Get(STAT_SPEED, STAT_ACTUAL);
    actualSpeed = new TextWidget(Rectangle(rect.GetXPos() + 68, rect.GetYPos() + 41, 12, 11), f);
    actualSpeed->SetText(actualSpeedStream.str());
    actualSpeed->SetAlignment(HA_RIGHT, VA_TOP);
    actualSpeed->SetColor(INFO_TEXT_COLOR);
    actualSpeed->SetShadow(COLOR_BLACK, 1, 1);
    std::stringstream actualStrengthStream;
    actualStrengthStream << std::setw(2) << std::setfill(' ') << playerCharacter->GetStatistics().Get(STAT_STRENGTH, STAT_ACTUAL);
    actualStrength = new TextWidget(Rectangle(rect.GetXPos() + 68, rect.GetYPos() + 52, 12, 11), f);
    actualStrength->SetText(actualStrengthStream.str());
    actualStrength->SetAlignment(HA_RIGHT, VA_TOP);
    actualStrength->SetColor(INFO_TEXT_COLOR);
    actualStrength->SetShadow(COLOR_BLACK, 1, 1);
    std::stringstream maximumHealthStream;
    maximumHealthStream << std::setw(2) << std::setfill(' ') << playerCharacter->GetStatistics().Get(STAT_HEALTH, STAT_MAXIMUM);
    maximumHealth = new TextWidget(Rectangle(rect.GetXPos() + 98, rect.GetYPos() + 19, 12, 11), f);
    maximumHealth->SetText(maximumHealthStream.str());
    maximumHealth->SetAlignment(HA_RIGHT, VA_TOP);
    maximumHealth->SetColor(INFO_TEXT_COLOR);
    maximumHealth->SetShadow(COLOR_BLACK, 1, 1);
    std::stringstream maximumStaminaStream;
    maximumStaminaStream << std::setw(2) << std::setfill(' ') << playerCharacter->GetStatistics().Get(STAT_STAMINA, STAT_MAXIMUM);
    maximumStamina = new TextWidget(Rectangle(rect.GetXPos() + 98, rect.GetYPos() + 30, 12, 11), f);
    maximumStamina->SetText(maximumStaminaStream.str());
    maximumStamina->SetAlignment(HA_RIGHT, VA_TOP);
    maximumStamina->SetColor(INFO_TEXT_COLOR);
    maximumStamina->SetShadow(COLOR_BLACK, 1, 1);
    condition = new TextWidget(Rectangle(rect.GetXPos() + 136, rect.GetYPos() + 19, 60, 11), f);
    switch (playerCharacter->GetCondition())
    {
    case COND_NORMAL:
        condition->SetText("Normal");
        break;
    case COND_SICK:
        condition->SetText("Sick");
        break;
    case COND_PLAGUED:
        condition->SetText("Plagued");
        break;
    case COND_POISONED:
        condition->SetText("Poisoned");
        break;
    case COND_DRUNK:
        condition->SetText("Drunk");
        break;
    case COND_HEALING:
        condition->SetText("Healing");
        break;
    case COND_STARVING:
        condition->SetText("Starving");
        break;
    case COND_NEAR_DEATH:
        condition->SetText("Near-death");
        break;
    default:
        condition->SetText("");
        break;
    }
    condition->SetAlignment(HA_LEFT, VA_TOP);
    condition->SetColor(INFO_TEXT_COLOR);
    condition->SetShadow(COLOR_BLACK, 1, 1);
}

RatingsWidget::~RatingsWidget()
{
    if (ratingsLabel)
    {
        delete ratingsLabel;
    }
    if (conditionLabel)
    {
        delete conditionLabel;
    }
    if (healthLabel)
    {
        delete healthLabel;
    }
    if (staminaLabel)
    {
        delete staminaLabel;
    }
    if (speedLabel)
    {
        delete speedLabel;
    }
    if (strengthLabel)
    {
        delete strengthLabel;
    }
    if (healthOfLabel)
    {
        delete healthOfLabel;
    }
    if (staminaOfLabel)
    {
        delete staminaOfLabel;
    }
    if (actualHealth)
    {
        delete actualHealth;
    }
    if (actualStamina)
    {
        delete actualStamina;
    }
    if (actualSpeed)
    {
        delete actualSpeed;
    }
    if (actualStrength)
    {
        delete actualStrength;
    }
    if (maximumHealth)
    {
        delete maximumHealth;
    }
    if (maximumStamina)
    {
        delete maximumStamina;
    }
    if (condition)
    {
        delete condition;
    }
}

void
RatingsWidget::SetBorders(Image *hb, Image *vb)
{
    if ((!hb) || (!vb))
    {
        throw NullPointer(__FILE__, __LINE__);
    }
    horizontalBorder = hb;
    verticalBorder = vb;
}

void
RatingsWidget::Draw()
{
    if (IsVisible())
    {
        int xoff = 0;
        int yoff = 0;
        if (horizontalBorder && verticalBorder)
        {
            horizontalBorder->Draw(rect.GetXPos() + verticalBorder->GetWidth(), rect.GetYPos(), 0, 0,
                                   rect.GetWidth() - 2 * verticalBorder->GetWidth(), horizontalBorder->GetHeight());
            horizontalBorder->Draw(rect.GetXPos() + verticalBorder->GetWidth(), rect.GetYPos() + rect.GetHeight() - horizontalBorder->GetHeight(), 0, 0,
                                   rect.GetWidth() - 2 * verticalBorder->GetWidth(), horizontalBorder->GetHeight());
            verticalBorder->Draw(rect.GetXPos(), rect.GetYPos(), 0, 0,
                                 verticalBorder->GetWidth(), rect.GetHeight());
            verticalBorder->Draw(rect.GetXPos() + rect.GetWidth() - verticalBorder->GetWidth(), rect.GetYPos(), 0, 0,
                                 verticalBorder->GetWidth(), rect.GetHeight());
            xoff = verticalBorder->GetWidth();
            yoff = horizontalBorder->GetHeight();
        }
        Video *video = MediaToolkit::GetInstance()->GetVideo();
        video->FillRect(rect.GetXPos() + xoff, rect.GetYPos() + yoff, rect.GetWidth() - 2 * xoff, rect.GetHeight() - 2 * yoff, 168);
        ratingsLabel->Draw();
        conditionLabel->Draw();
        healthLabel->Draw();
        staminaLabel->Draw();
        speedLabel->Draw();
        strengthLabel->Draw();
        healthOfLabel->Draw();
        staminaOfLabel->Draw();
        actualHealth->Draw();
        actualStamina->Draw();
        actualSpeed->Draw();
        actualStrength->Draw();
        maximumHealth->Draw();
        maximumStamina->Draw();
        condition->Draw();
    }
}

void
RatingsWidget::Drag(const int, const int)
{}

void
RatingsWidget::Drop(const int, const int)
{}
