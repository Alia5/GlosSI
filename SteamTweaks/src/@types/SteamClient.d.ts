export interface SteamClient {
    Settings: {
        SetInGameOverlayShowFPSCorner: (value: 0|1|2|3|4) => void;
        SetInGameOverlayShowFPSContrast: (value: boolean) => void;
    };
}

declare global {
    interface Window {
        SteamClient: SteamClient;
    }
    // eslint-disable-next-line
    declare const SteamClient: SteamClient;
}
