import type { SteamConfig } from '../../../common/util/types';
import { initTweak } from '../../../common/tweakApi';


const backup: { originalFpsCorner?: number } = {};
initTweak('HideFPSCounter', {
    install: async () => {
        backup.originalFpsCorner = Number(
            ((await GlosSIApi.SteamTarget.getSteamSettings()).system as SteamConfig)
                .InGameOverlayShowFPSCorner
        ) as 0 | 1 | 2 | 3 | 4;
        if (!SteamClient.Settings.SetInGameOverlayShowFPSCorner) {
            GlosSIApi.SteamTarget.log('warn',
                'HideFPSCounter: SteamClient.Settings.SetInGameOverlayShowFPSCorner is not available.'
                +'Can\'t hide FPS Counter corner.'
            );
        }
        SteamClient.Settings.SetInGameOverlayShowFPSCorner?.(0);
    },
    uninstall: () => {
        if (!SteamClient.Settings.SetInGameOverlayShowFPSCorner) {
            return;
        }
        GlosSIApi.SteamTarget.log('debug','uninstalling HideFPSCounter Tweak. Restoring FPS Counter corner: ', backup.originalFpsCorner);
        SteamClient.Settings.SetInGameOverlayShowFPSCorner?.((backup.originalFpsCorner ?? 0) as 0 | 1 | 2 | 3 | 4);
        setTimeout(() => {
            // steam might not actually register the setting?! Try again like 10 seconds later... ¯\_(ツ)_/¯
            SteamClient.Settings.SetInGameOverlayShowFPSCorner?.((backup.originalFpsCorner ?? 0) as 0 | 1 | 2 | 3 | 4);
        }, 10 * 1000);
    }
}).then(() => {
    GlosSIApi.SteamTarget.log('debug', 'HideFPSCounter installed');
}).catch((e) => GlosSIApi.SteamTarget.log('error', 'HideFPSCounter failed to install', e));
