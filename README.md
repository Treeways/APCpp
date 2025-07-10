# APCppWrapped
C++ Library for Clients interfacing with the [Archipelago Multi-Game Randomizer](https://archipelago.gg)

## Differences from APCpp

APCppWrapped is a simple fork of APCpp. It wraps APCpp's methods and members in an `Archipelago` class.

The goal is to provide additional encapsulation.
Because of this, it overcomes a limitation of the original library, which makes it possible to
start more than one connection at once within an app.
(If you do not need these features, I would recommend sticking with [the original library](https://github.com/N00byKing/APCpp)
for simplicity's sake, and for the latest updates.)

This library does not add additional functionality. The API is identical, other than the class wrapper.
The code has been rearranged, though it's in no way significantly different than the original implementation
(other than `#include` statements, constants and members being moved into the header file for abstraction reasons).

## Caveats

- APCppWrapped is very intentionally designed to be a simple wrapper, and nothing more.
There are no new features, nor is there a constructor or destructor, nor are there any new virtual functions or overrides.
This minimizes API differences, and makes code maintenance easier too.
With that said, please remember to call `AP_Shutdown()` before destroying your class instance, or letting it go out of scope.
- The main use case for this wrapper is for managing multiple slots in one application. That also means slots are decoupled from one another.
If you know you'll be handling multiple connections to the same multiworld, then you might need to manage slots
within your own `Multiworld` struct, ie. to prevent the possibility of double-gets, or even race conditions.
(For instance, if you need to fetch a server datapackage, then you should fetch it once per multiworld,
rather than fetching multiple datapackages.)

# Usage

## Example

```cpp
Archipelago *slot1 = new Archipelago();
// Now you can start a second connection if you need to!
Archipelago *slot2 = new Archipelago();

slot1->AP_Init("archipelago.gg:12345", "MyCoolApp", "Player", "");
slot1->AP_SetItemClearCallback(&itemclear);
slot1->AP_SetItemRecvCallback(&itemrecv);
slot1->AP_SetLocationCheckedCallback(&locchk);
// and so on...

slot1->AP_Start();
// then, when you're done...
slot1->AP_Disconnect();
delete slot1;
```

## Initialization

First, instantiate the `Archipelago` class.
- `Archipelago *slot = new Archipelago();`

From here on out, it's implied that you use pointer member selection, ie. `slot->AP_Init()`.

Then, run one of the `Archipelago::AP_Init` functions as the first call to the library:
- `AP_Init(const char*, const char*, const char*, const char*)` with IP, Game Name, Slot Name and password (can be `""`)
- `AP_Init(const char*)` with the filename corresponding to a generated single player game.

Then, you must call the following functions (any order):
- `AP_SetItemClearCallback(void (*f_itemclr)())` with a callback that clears all item states
- `AP_SetItemRecvCallback(void (*f_itemrecv)(int,bool))` with a callback that adds the item with the given ID (first parameter) to the current state.
The secound parameter decides whether or not to notify the player
- `AP_SetLocationCheckedCallback(void (*f_locrecv)(int))` with a callback that marks the location with the given id (first parameter) as checked.

Optionally, for finer configuration:
- `AP_EnableQueueItemRecvMsgs(bool)` Enables or disables Item Messages for Items received for the current game. Alternative to using the game's native item reception handler, if present. Defaults to on.

Optionally, for DeathLink:
- `AP_SetDeathLinkSupported(bool)` Enables or disables DeathLink from the Library. Defaults to off. NOTE: If on, expects DeathLink data from Archipelago.
- `AP_SetDeathLinkRecvCallback(void (*f_deathrecv)())` Alternative to manual query. Optional callback to handle DeathLink.

Optionally, if slot data is required:
- `AP_RegisterSlotDataIntCallback(std::string, void (*f_slotdata)(int))` Add a callback that receives an int from slot data with the first parameter as its key.
- `AP_RegisterSlotDataMapIntIntCallback(std::string, void (*f_slotdata)(std::map<int,int>))` Add a callback that receives an int to int map from slot data with the first parameter as its key.

Finally, call `AP_Start()`

## Operation during runtime

When the player completes a check, call `AP_SendItem(int)` with the Item ID as the parameter.
When the player completes the game, call `AP_StoryComplete`.

### DeathLink
If DeathLink is supported, you have multiple ways of using it:
- Regularly call `AP_DeathLinkPending()` and check the return value. If true, kill the player and call `AP_DeathLinkClear()` (Preferably after a short time.
Faulty clients can send multiple deaths in quick succession. Ex.: Clear only after player has recovered from death).
- Handle death using a DeathLink callback (Registration described in [Initialization](#Initialization))

### Messages
Messages can be received from the Archipelago Server and this Library, such as Messages describing which item was sent, who was responsible for a Death received with DeathLink, etc.
To receive messages:
- Check if message is available using `AP_IsMessagePending()`
- Receive with `AP_GetLatestMessage()`. This returns an AP_Message struct with a type and preconfigured presentable text for this message. If you want the game client to have more details for the message (for example to create a custom text) any non-plaintext type message can be casted to an AP_`TYPE`Message with some type-specific struct members providing additional information for the given message.
- Clear the latest message with `AP_ClearLatestMessage()`.

# Building
Clone the Repo recursively!
## Linux
- Create a folder `build`
- `cd build`
- `cmake ..`
- `cmake --build .`
## Windows
- Create a folder `build`
- Enter the folder
- `cmake .. -DWIN32=1` (If on MinGW, also add `-DMINGW=1`)
- `cmake --build .`
