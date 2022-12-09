## Описание работы 

  При запуске приложения один из игроков нажимает 1, что перемещает его на карту (Lobby) и создает Listen сервер. Второй игрок нажимает 2, что перемещает его к первому игроку который для него является сервером. 
  
  Для подключения используется открытый Steam Dev ID (SteamDevAppId=480)

  Для работы необходимо чтобы у каждого отдельного подключения было включено отдельное приложение Steam 
  
## Сборка проекта 
 
Для сборки необходимо соблюсти следующие условия: 
  - Версия Unreal Engine 5.1 (UE)
  - Внутри редактора UE подключен плагин Online Subsytem Steam 
  - В файле [DefaultEngine.ini](https://github.com/Kirill-Geskin/MP_Plugin/blob/main/Config/DefaultEngine.ini) добавленны  
  
      ```c++
      [OnlineSubsystem]
    DefaultPlatformService=Steam

    [OnlineSubsystemSteam]
    bEnabled=true
    SteamDevAppId=480
    bInitServerOnClient=true

    [/Script/OnlineSubsystemSteam.SteamNetDriver]
    NetConnectionClassName="OnlineSubsystemSteam.SteamNetConnection"
      ```

  - В файле [MenuSystem.Build.cs](https://github.com/Kirill-Geskin/MP_Plugin/blob/main/Source/MenuSystem/MenuSystem.Build.cs) подключены дополнительные модули "OnlineSubsystemSteam", "OnlineSubsystem" 
  - В редакторе UE в блюпринте персонажа вызваны функции 
  
      ![SharedScreenshot](https://user-images.githubusercontent.com/80645926/206653279-f7436b08-313c-49d7-86ae-5fa432171143.jpg)

