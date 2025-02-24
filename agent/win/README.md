## Design
```mermaid
%% mermaid code
flowchart TD
%% Sophon Agent v0.0.1
    %% started
    SS1[flag: bool started = true]
    %% paused
    SS0[flag: bool started = false] 
    %% authenticated
    SA1[flag: bool authenticated = true]
    %% not authenticated
    SA0[flag: bool authenticated = false]

    EXECUTE([execute Sophon Agent])-->SA0-->SS0-->MENU{user selects the menu}
    MENU-->|close|TERMINATE([terminate Sophon Agent])
    MENU-->|login|LOGIN[/input: user name, remote storage addr/]-->LOGIN1[collect device name, IP addr, MAC addr, number of monitors]-->SA1
    MENU-->|loout|LOOUT[flag: bool authenticated = false]
    MENU-->|pause|SS0
    MENU-->|start|SS1-->|true|FLAG_INQUERY{flag: authenticated && started}-->|true|SCREENSHOT{{capture screenshot, send the screenshot to remote storage addr}}-->SCREENSHOT1[wait 10s]-->FLAG_INQUERY
```
## TODO
- Sending the screenshot to remote storages (GCP Storage, AWS S3, Google Drive, NFS, on-premise repo)
- Remote storage authentication
- Shoot screenshot to png instead of bmp
- Screenshot encryption
- Screenshot compression
- Screenshot per monitor
- Low level screenshot instead of GDI, in order to shoot DRM/secure programs
## Changelog
#### v0.1.0
###### Framework
- Win32API: For precise system control, Faster & Simpler code than MFC (MFC is not pretty & difficult for Bin)
- Visual Studio 2022
###### Features
- Window minimization as a system tray icon
- 4 buttons: Login, Logout, Start, Pause
- 2 indicators: Auth status (login/logout), Run status (start/pause)
- Capturing bmp screenshot per 10s with Windows GDI functions
- Save the screenshots in the same path where the sophon-agent.exe locates
- Added curl libraries
