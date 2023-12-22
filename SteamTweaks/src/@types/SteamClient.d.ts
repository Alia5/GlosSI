export interface SteamClient {
    Settings: {
        // Current stable (As time of commit); Beta does not have this anymore...
        SetInGameOverlayShowFPSCorner?: (value: 0|1|2|3|4) => void;
        SetInGameOverlayShowFPSContrast?: (value: boolean) => void;
        // TODO: find a way to change setting on beta (and soon stable...)
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
