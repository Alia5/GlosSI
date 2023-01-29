
import type { SteamConfig } from "../../common/util/types";
import { initTweak } from "../../common/tweakApi";

// variables here are scoped to the tweak
// and are not accessible from other tweaks or even the main script
// there is no risk of conflicts


const originalFpsCorner = Number(
    ((await GlosSI.getSettings()).system as SteamConfig)
        .InGameOverlayShowFPSCorner
) as 0 | 1 | 2 | 3 | 4;

initTweak('HideFPSCounter', {
    install: () => {
        SteamClient.Settings.SetInGameOverlayShowFPSCorner(0);
    },
    uninstall: () => {
        SteamClient.Settings.SetInGameOverlayShowFPSCorner(originalFpsCorner);
    }
});
