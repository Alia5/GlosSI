export interface SteamClient {
    Settings: {
        SetInGameOverlayShowFPSCorner: (value: 0|1|2|3|4) => void;
        SetInGameOverlayShowFPSContrast: (value: boolean) => void;
    };
    UI: {
        GetUiMode: () => Promise<SteamUiMode>;
        SetUiMode: (mode: SteamUiMode) => void;
    };
    Window: {
        Minimize();
        HideWindow();
    };
}

export type FullSteamClient = Required<SteamClient>;

declare global {
    interface Window {
        SteamClient: SteamClient;
    }
    // eslint-disable-next-line
    declare const SteamClient: SteamClient;
}
