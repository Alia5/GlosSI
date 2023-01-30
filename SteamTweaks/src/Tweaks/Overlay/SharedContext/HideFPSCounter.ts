
import type { SteamConfig } from "../../../common/util/types";
import { initTweak } from "../../../common/tweakApi";


const backup: { originalFpsCorner?: number } = {};
initTweak('AnotherTweak', {
    install: async () => {
        backup.originalFpsCorner = Number(
            ((await GlosSIApi.SteamTarget.getSteamSettings()).system as SteamConfig)
                .InGameOverlayShowFPSCorner
        ) as 0 | 1 | 2 | 3 | 4;
        SteamClient.Settings.SetInGameOverlayShowFPSCorner(0);
    },
    uninstall: () => {
        SteamClient.Settings.SetInGameOverlayShowFPSCorner((backup.originalFpsCorner ?? 0) as 0 | 1 | 2 | 3 | 4);
    }
});
