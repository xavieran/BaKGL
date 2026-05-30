function init()
{
    var dialogResultEnum = enumeration("DialogResult", uint16(), {
        "SetTextVariable": 0x01,
        "GiveItem": 0x02,
        "LoseItem": 0x03,
        "SetFlag": 0x04,
        "LoadActor": 0x05,
        "SetPopupDimensions": 0x06,
        "SpecialAction": 0x07,
        "GainCondition": 0x08,
        "GainSkill": 0x09,
        "LoadSkillValue": 0x0a,
        "PlaySound": 0x0c,
        "ElapseTime": 0x0d,
        "SetAddResetState": 0x0e,
        "FreeMemoryP": 0x0f,
        "PushNextDialog": 0x10,
        "UpdateCharacters": 0x11,
        "HealCharacters": 0x12,
        "LearnSpell": 0x13,
        "Teleport": 0x14,
        "SetEndOfDialogState": 0x15,
        "SetTimeExpiringState": 0x16,
        "LoseNOfItem": 0x17,
    });

    var specialActionTypeEnum = enumeration("SpecialActionType", uint16(), {
        "ReduceGold": 0,
        "IncreaseGold": 1,
        "RepairAllEquippedArmor": 2,
        "ReactivateCombat": 3,
        "DeactivateCombat": 4,
        "CopyStandardInnToShop0": 5,
        "CopyStandardInnToShop1": 6,
        "Increase753f": 7,
        "Gamble": 8,
        "RepairAndBlessEquippedSwords": 9,
        "ReturnAlcoholToShops": 10,
        "ResetGambleValueTo": 11,
        "BeginCombat": 12,
        "ExtinguishAllLightSources": 13,
        "EmptyArlieContainer": 14,
        "CheatIncreaseSkill": 15,
        "UnifyOwynAndPugsSpells": 16,
    });

    var conditionEnum = enumeration("Condition", uint16(), {
        "Sick": 0,
        "Plagued": 1,
        "Poisoned": 2,
        "Drunk": 3,
        "Healing": 4,
        "Starving": 5,
        "NearDeath": 6,
    });

    var setTextVariable = alternative(
        function() { return this.Type.uint16 == 0x01; },
        { Which: uint16(), What: uint16(), Rest: array(uint8(), 4) },
        "SetTextVariable");

    var giveItem = alternative(
        function() { return this.Type.uint16 == 0x02; },
        { Item: uint8(), Character: uint8(), Quantity: uint16(), Rest: array(uint8(), 4) },
        "GiveItem");

    var loseItem = alternative(
        function() { return this.Type.uint16 == 0x03; },
        { Item: uint16(), Quantity: uint16(), Rest: array(uint8(), 4) },
        "LoseItem");

    var setFlag = alternative(
        function() { return this.Type.uint16 == 0x04; },
        { EventPtr: uint16(), Mask: uint8(), Data: uint8(), AlwaysZero: uint16(), Value: uint16() },
        "SetFlag");

    var loadActor = alternative(
        function() { return this.Type.uint16 == 0x05; },
        { Actor1: uint16(), Actor2: uint16(), Actor3: uint16(), Unknown: uint16() },
        "LoadActor");

    var setPopupDimensions = alternative(
        function() { return this.Type.uint16 == 0x06; },
        { PosX: uint16(), PosY: uint16(), DimsX: uint16(), DimsY: uint16() },
        "SetPopupDimensions");

    var specialAction = alternative(
        function() { return this.Type.uint16 == 0x07; },
        { ActionType: specialActionTypeEnum, Var1: uint16(), Var2: uint16(), Var3: uint16() },
        "SpecialAction");

    var gainCondition = alternative(
        function() { return this.Type.uint16 == 0x08; },
        { Who: uint16(), Condition: conditionEnum, Min: int16(), Max: int16() },
        "GainCondition");

    var gainSkill = alternative(
        function() { return this.Type.uint16 == 0x09; },
        { Who: uint16(), Skill: uint16(), Min: int16(), Max: int16() },
        "GainSkill");

    var loadSkillValue = alternative(
        function() { return this.Type.uint16 == 0x0a; },
        { Target: uint16(), Skill: uint16(), Padding: array(uint8(), 4) },
        "LoadSkillValue");

    var playSound = alternative(
        function() { return this.Type.uint16 == 0x0c; },
        { SoundIndex: uint16(), Flag: uint16(), Rest: uint32() },
        "PlaySound");

    var elapseTime = alternative(
        function() { return this.Type.uint16 == 0x0d; },
        { Time: uint32(), Rest: array(uint8(), 4) },
        "ElapseTime");

    var setAddResetState = alternative(
        function() { return this.Type.uint16 == 0x0e; },
        { State: uint16(), Unknown0: uint16(), TimeToExpire: uint32() },
        "SetAddResetState");

    var pushNextDialog = alternative(
        function() { return this.Type.uint16 == 0x10; },
        { Target: uint32(), Rest: array(uint8(), 4) },
        "PushNextDialog");

    var updateCharacters = alternative(
        function() { return this.Type.uint16 == 0x11; },
        { Count: uint16(), Char1: uint16(), Char2: uint16(), Char3: uint16() },
        "UpdateCharacters");

    var healCharacters = alternative(
        function() { return this.Type.uint16 == 0x12; },
        { Who: uint16(), HowMuch: uint16(), Padding: array(uint8(), 4) },
        "HealCharacters");

    var learnSpell = alternative(
        function() { return this.Type.uint16 == 0x13; },
        { Who: uint16(), WhichSpell: uint16(), Rest: array(uint8(), 4) },
        "LearnSpell");

    var teleport = alternative(
        function() { return this.Type.uint16 == 0x14; },
        { TeleportIndex: uint16(), Padding: array(uint8(), 6) },
        "Teleport");

    var setEndOfDialogState = alternative(
        function() { return this.Type.uint16 == 0x15; },
        { State: int16(), Rest: array(uint8(), 6) },
        "SetEndOfDialogState");

    var setTimeExpiringState = alternative(
        function() { return this.Type.uint16 == 0x16; },
        { Type: uint8(), Flags: uint8(), EventPtr: uint16(), TimeToExpire: uint32() },
        "SetTimeExpiringState");

    var loseNOfItem = alternative(
        function() { return this.Type.uint16 == 0x17; },
        { Item: uint16(), Quantity: uint16(), Rest: array(uint8(), 4) },
        "LoseNOfItem");

    var dialogAction = taggedUnion(
        { Type: dialogResultEnum },
        [
            setTextVariable,
            giveItem,
            loseItem,
            setFlag,
            loadActor,
            setPopupDimensions,
            specialAction,
            gainCondition,
            gainSkill,
            loadSkillValue,
            playSound,
            elapseTime,
            setAddResetState,
            pushNextDialog,
            updateCharacters,
            healCharacters,
            learnSpell,
            teleport,
            setEndOfDialogState,
            setTimeExpiringState,
            loseNOfItem,
        ]);

    var dialogChoice = struct({
        State: uint16(),
        Min: uint16(),
        Max: uint16(),
        Target: uint32(),
    });

    var dialogSnippet = struct({
        DisplayStyle: uint8(),
        Actor: uint16(),
        DisplayStyle2: uint8(),
        DisplayStyle3: uint8(),
        NumChoices: uint8(),
        NumActions: uint8(),
        TextLength: uint16(),
        Choices: array(dialogChoice, function(s) { return s.NumChoices.uint8; }),
        Actions: array(dialogAction, function(s) { return s.NumActions.uint8; }),
        Text: array(char(), function(s) { return s.TextLength.uint16; }),
    });

    return dialogSnippet;
}
