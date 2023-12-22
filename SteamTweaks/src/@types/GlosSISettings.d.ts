export interface GlosSISettings {
    controller: {
        allowDesktopConfig: boolean;
        emulateDS4: boolean;
        maxControllers: number;
    };
    devices: {
        hideDevices: boolean;
        realDeviceIds: boolean;
    };
    extendedLogging: boolean;
    globalModeGameId: string;
    globalModeUseGamepadUI: boolean;
    icon?: string;
    ignoreEGS: boolean;
    killEGS: boolean;
    launch: {
        closeOnExit: boolean;
        ignoreLauncher: boolean;
        killLauncher: boolean;
        launch: boolean;
        launchAppArgs?: string;
        launchPath?: string;
        launcherProcesses: string[];
        waitForChildProcs: boolean;
    };
    name?: string;
    snapshotNotify: boolean;
    standaloneModeGameId: string;
    standaloneUseGamepadUI: boolean;
    minimizeSteamGamepadUI: boolean;
    steamPath: string;
    steamUserId: string;
    steamgridApiKey: string;
    version: number;
    window: {
        disableGlosSIOverlay: boolean;
        disableOverlay: boolean;
        hideAltTab: boolean;
        maxFps?: number;
        scale?: number;
        windowMode: boolean;
    };
}
