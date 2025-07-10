#pragma once

#include "ixwebsocket/IXNetSystem.h"
#include "ixwebsocket/IXWebSocket.h"
#include "ixwebsocket/IXUserAgent.h"

#include <json/json.h>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

#include <cstddef>
#include <cstdint>
#include <chrono>
#include <deque>
#include <fstream>
#include <functional>
#include <map>
#include <random>
#include <set>
#include <string>
#include <utility>
#include <vector>

#define AP_PERMISSION_DISABLED 0b000
#define AP_PERMISSION_ENABLED 0b001
#define AP_PERMISSION_GOAL 0b010
#define AP_PERMISSION_AUTO 0b110

struct AP_NetworkVersion
{
    int major;
    int minor;
    int build;
};

struct AP_NetworkItem
{
    int64_t item;
    int64_t location;
    int player;
    int flags;
    std::string itemName;
    std::string locationName;
    std::string playerName;
};

struct AP_NetworkPlayer
{
    int team;
    int slot;
    std::string name;
    std::string alias;
    std::string game;
};

/* Message Management Types */

enum struct AP_MessageType
{
    Plaintext,
    ItemSend,
    ItemRecv,
    Hint,
    Countdown
};

struct AP_Message
{
    AP_MessageType type = AP_MessageType::Plaintext;
    std::string text;
};

struct AP_ItemSendMessage : AP_Message
{
    std::string item;
    std::string recvPlayer;
};

struct AP_ItemRecvMessage : AP_Message
{
    std::string item;
    std::string sendPlayer;
};

struct AP_HintMessage : AP_Message
{
    std::string item;
    std::string sendPlayer;
    std::string recvPlayer;
    std::string location;
    bool checked;
};

struct AP_CountdownMessage : AP_Message
{
    int timer;
};

/* Connection Information Types */

enum struct AP_ConnectionStatus
{
    Disconnected,
    Connected,
    Authenticated,
    ConnectionRefused
};

struct AP_RoomInfo
{
    AP_NetworkVersion version;
    std::vector<std::string> tags;
    bool password_required;
    std::map<std::string, int> permissions;
    int hint_cost;
    int location_check_points;
    // MISSING: games
    std::map<std::string, std::string> datapackage_checksums;
    std::string seed_name;
    double time;
};

/* Serverside Data Types */

enum struct AP_RequestStatus
{
    Pending,
    Done,
    Error
};

enum struct AP_DataType
{
    Raw,
    Int,
    Double
};

struct AP_GetServerDataRequest
{
    AP_RequestStatus status;
    std::string key;
    void *value;
    AP_DataType type;
};

struct AP_DataStorageOperation
{
    std::string operation;
    void *value;
};

struct AP_SetServerDataRequest
{
    AP_RequestStatus status;
    std::string key;
    std::vector<AP_DataStorageOperation> operations;
    void *default_value;
    AP_DataType type;
    bool want_reply;
};

struct AP_SetReply
{
    std::string key;
    void *original_value;
    void *value;
};

struct AP_Bounce
{
    std::vector<std::string> *games = nullptr; // Can be nullptr or empty, but must be set to either
    std::vector<std::string> *slots = nullptr; // Can be nullptr or empty, but must be set to either
    std::vector<std::string> *tags = nullptr;  // Can be nullptr or empty, but must be set to either
    std::string data;                          // Valid JSON Data. Can also be primitive (Numbers or literals)
};

constexpr int AP_OFFLINE_SLOT = 1404;
constexpr char const* AP_OFFLINE_NAME = "You";
constexpr AP_NetworkVersion AP_DEFAULT_NETWORK_VERSION = {0,5,1}; // Default for compatibility reasons

class Archipelago
{
public:
    void AP_Init(const char *, const char *, const char *, const char *);
    void AP_Init(const char *);
    bool AP_IsInit();

    void AP_Start();

    // AP_Shutdown resets the library state to before initialization, and doesn't just disconnect!
    void AP_Shutdown();

    // Set current client version
    void AP_SetClientVersion(AP_NetworkVersion *);

    /* Configuration Functions */

    void AP_EnableQueueItemRecvMsgs(bool);

    void AP_SetDeathLinkSupported(bool);

    /* Required Callback Functions */

    // Parameter Function must reset local state
    void AP_SetItemClearCallback(std::function<void()> f_itemclr);
    // Parameter Function must collect item id given with parameter. Secound parameter indicates whether or not to notify player
    void AP_SetItemRecvCallback(std::function<void(int64_t, bool)> f_itemrecv);
    // Parameter Function must mark given location id as checked
    void AP_SetLocationCheckedCallback(std::function<void(int64_t)> f_locrecv);

    /* Optional Callback Functions */

    // Parameter Function will be called when Death Link is received. Alternative to Pending/Clear usage
    void AP_SetDeathLinkRecvCallback(std::function<void()> f_deathrecv);
    // Overload with the deathlink source and cause
    void AP_SetDeathLinkRecvCallback(std::function<void(std::string, std::string)> f_deathrecv);

    // Parameter Function receives Slotdata of respective type
    void AP_RegisterSlotDataIntCallback(std::string, std::function<void(int)> f_slotdata);
    void AP_RegisterSlotDataMapIntIntCallback(std::string, std::function<void(std::map<int, int>)> f_slotdata);
    void AP_RegisterSlotDataRawCallback(std::string, std::function<void(std::string)> f_slotdata);

    // Send LocationScouts packet
    void AP_SendLocationScouts(std::set<int64_t> const &locations, int create_as_hint);
    // Receive Function for LocationInfo
    void AP_SetLocationInfoCallback(std::function<void(std::vector<AP_NetworkItem>)> f_locinfrecv);

    /* Game Management Functions */

    // Sends LocationCheck for given index
    void AP_SendItem(int64_t location);
    void AP_SendItem(std::set<int64_t> const &locations);

    // Called when Story completed, sends StatusUpdate
    void AP_StoryComplete();

    /* Deathlink Functions */

    bool AP_DeathLinkPending();
    void AP_DeathLinkClear();
    void AP_DeathLinkSend();

    /* Message Management Functions */

    bool AP_IsMessagePending();
    void AP_ClearLatestMessage();
    AP_Message *AP_GetLatestMessage();

    void AP_Say(std::string);

    /* Connection Information Functions */

    int AP_GetRoomInfo(AP_RoomInfo *);
    AP_ConnectionStatus AP_GetConnectionStatus();
    std::uint64_t AP_GetUUID();
    int AP_GetPlayerID();

    /* Serverside Data Functions */

    // Set and Receive Data
    void AP_SetServerData(AP_SetServerDataRequest *request);
    void AP_GetServerData(AP_GetServerDataRequest *request);

    // This returns a string prefix, consistent across game connections and unique to the player slot.
    // Intended to be used for getting / setting private server data
    // No guarantees are made regarding the content of the prefix!
    std::string AP_GetPrivateServerDataPrefix();

    // Parameter Function receives all SetReply's
    // ! Pointers in AP_SetReply struct only valid within function !
    // If values are required beyond that a copy is needed
    void AP_RegisterSetReplyCallback(std::function<void(AP_SetReply)> f_setreply);

    // Receive all SetReplys with Keys in parameter list
    void AP_SetNotify(std::map<std::string, AP_DataType>);
    // Single Key version of above for convenience
    void AP_SetNotify(std::string, AP_DataType);

    // Send Bounce package
    void AP_SendBounce(AP_Bounce);

    // Receive Bounced packages. Disables automatic DeathLink management
    void AP_RegisterBouncedCallback(std::function<void(AP_Bounce)> f_bounced);

private:
    bool init = false;
    bool auth = false;
    bool refused = false;
    bool multiworld = true;
    bool isSSL = true;
    bool ssl_success = false;
    int ap_player_id;
    std::string ap_player_name;
    size_t ap_player_name_hash;
    std::string ap_ip;
    std::string ap_game;
    std::string ap_passwd;
    std::uint64_t ap_uuid = 0;
    std::mt19937 rando;
    AP_NetworkVersion client_version = AP_DEFAULT_NETWORK_VERSION;

    // Deathlink Stuff
    bool deathlinkstat = false;
    bool deathlinksupported = false;
    bool enable_deathlink = false;
    int deathlink_amnesty = 0;
    int cur_deathlink_amnesty = 0;

    // Message System
    std::deque<AP_Message *> messageQueue;
    bool queueitemrecvmsg = true;

    // Data Maps
    std::map<int, AP_NetworkPlayer> map_players;
    std::map<std::pair<std::string, int64_t>, std::string> map_location_id_name;
    std::map<std::pair<std::string, int64_t>, std::string> map_item_id_name;

    // Callback function pointers
    std::function<void()> resetItemValues = nullptr;
    std::function<void(int64_t, bool)> getitemfunc = nullptr;
    std::function<void(int64_t)> checklocfunc = nullptr;
    std::function<void(std::vector<AP_NetworkItem>)> locinfofunc = nullptr;
    std::function<void(std::string, std::string)> recvdeath = nullptr;
    std::function<void(AP_SetReply)> setreplyfunc = nullptr;
    std::function<void(AP_Bounce)> bouncedfunc = nullptr;

    // Serverdata Management
    std::map<std::string, AP_DataType> map_serverdata_typemanage;
    AP_GetServerDataRequest resync_serverdata_request;
    uint64_t last_item_idx = 0;

    // Singleplayer Seed Info
    std::string sp_save_path;
    Json::Value sp_save_root;

    // Misc Data for Clients
    AP_RoomInfo lib_room_info;

    // Server Data Stuff
    std::map<std::string, AP_GetServerDataRequest *> map_server_data;

    // Slot Data Stuff
    std::map<std::string, std::function<void(int)>> map_slotdata_callback_int;
    std::map<std::string, std::function<void(std::string)>> map_slotdata_callback_raw;
    std::map<std::string, std::function<void(std::map<int, int>)>> map_slotdata_callback_mapintint;
    std::vector<std::string> slotdata_strings;

    // Datapackage Stuff
    std::string const datapkg_cache_path = "APCpp_datapkg.cache";
    Json::Value datapkg_cache;
    std::set<std::string> datapkg_outdated_games;

    ix::WebSocket webSocket;
    Json::Reader reader;
    Json::FastWriter writer;

    Json::Value sp_ap_root;

    void AP_Init_Generic();
    bool parse_response(std::string msg, std::string &request);
    void APSend(std::string req);
    void WriteFileJSON(Json::Value val, std::string path);
    std::string getItemName(std::string game, int64_t id);
    std::string getLocationName(std::string game, int64_t id);
    void parseDataPkg(Json::Value new_datapkg);
    void parseDataPkg();
    AP_NetworkPlayer getPlayer(int team, int slot);
};
