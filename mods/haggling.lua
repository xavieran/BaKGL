-- Haggling dialogue mod
-- Called when the player successfully haggles.
-- gs           — GameState usertype with direct C++ API access.
-- discount_pct — Percentage discount received (0-100).
-- Return a dialog key (number) to show, or nil for default C++ behaviour.
function on_haggle_success(gs, discount_pct)
    if discount_pct >= 50 then return 0x1B8000 end
    if discount_pct >= 25 then return 0x1B8001 end
    if discount_pct >= 15 then return 0x1B8002 end
    if discount_pct >= 10 then return 0x1B8003 end
    if discount_pct >= 5  then return 0x1B8004 end
    return 0x1B8005
end

-- Called when haggling fails or the shopkeeper refuses.
-- gs       — GameState usertype with direct C++ API access.
-- item_type — The ItemType enum value (Sword = 1).
-- Return a dialog key (number) to show, or nil for the game's default.
function on_haggle_fail(gs, item_type)
    if item_type == ItemType.Sword then
        return 0x1BA001
    end
    return 0x1BA000
end

-- Called when the player tries to haggle a scroll.
-- Return a dialog key (number) to show, or nil for the game's default.
function on_cant_haggle_scroll(gs)
    return 0x1b775f
end
