#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "discord_rpc.h"

/* shutdown discord-rpc */
void shutdown_discord(int sig) {
  printf("[Info] shutting down...\n");
  Discord_Shutdown();
  exit(sig);
}

/* handle discord ready event */
static void handleDiscordReady(const DiscordUser* u) {
  printf("[Discord] Displaying Presence for %s#%s\n", u->username,
         u->discriminator);
}

/* handle discord disconnected event */
static void handleDiscordDisconnected(int errcode, const char* message) {
  printf("[Discord] disconnected (%d: %s)\n", errcode, message);
}

/* handle discord error event */
static void handleDiscordError(int errcode, const char* message) {
  printf("[Discord] error (%d: %s)\n", errcode, message);
}

/* print values for the current settings from the config file */
static void print_config_values(const DiscordRichPresence* d,
                                const char* client_id) {
  printf("Current Presence (%s):\n", client_id);
  printf("State: %s\n", d->state);
  printf("Details: %s\n", d->details);
  printf("Large Image: '%s' with toolip, '%s'\n", d->largeImageKey,
         d->largeImageText);
  printf("Small Image: '%s' with toolip, '%s'\n", d->smallImageKey,
         d->smallImageText);
  printf("Start Time: %lld\n", d->startTimestamp);
  printf("End Time: %lld\n", d->endTimestamp);
}

/* update discord presence */
void update_presence(struct ini_parser* p) {
  /* object from discord-rpc api */
  DiscordRichPresence discordPresence;
  memset(&discordPresence, 0, sizeof(discordPresence));

  /* check required variables */
  char* state = get_ini_value(p, "State", "State");
  if (strlen(state) < 1) {
    printf("[Warning] State parameter is not set\n");
    return;
  }
  discordPresence.state = state;
  char* details = get_ini_value(p, "State", "Details");
  if (strlen(details) < 1) {
    printf("[Warning] Details parameter is not set\n");
    return;
  }
  discordPresence.details = details;
  char* large_image_key = get_ini_value(p, "Images", "LargeImage");
  if (strlen(large_image_key) < 1) {
    printf("[Warning] LargeImage parameter is not set\n");
    return;
  }
  discordPresence.largeImageKey = large_image_key;

  int64_t start_time = strtoll(get_ini_value(p, "State", "StartTimestamp"), NULL, 10);
  int64_t end_time = strtoll(get_ini_value(p, "State", "EndTimestamp"), NULL, 10);
  if (start_time == LLONG_MAX || start_time == LLONG_MIN) {
    printf("[Error] StartTimestamp value is not a valid number\n");
    return;
  }
  if (end_time == LLONG_MAX || end_time == LLONG_MIN) {
    printf("[Error] EndTimestamp value is not a valid number\n");
    return;
  }

  discordPresence.startTimestamp = start_time;
  discordPresence.endTimestamp = end_time;

  /* dont set optional variables if they are not defined in the config */
  char* small_image_key = get_ini_value(p, "Images", "SmallImage");
  if (strlen(small_image_key) >= 1)
    discordPresence.smallImageKey = small_image_key;
  char* small_image_text = get_ini_value(p, "Images", "SmallImageTooltip");
  if (strlen(small_image_text) >= 1)
    discordPresence.smallImageText = small_image_text;
  char* large_image_text = get_ini_value(p, "Images", "LargeImageTooltip");
  if (strlen(large_image_text) >= 1)
    discordPresence.largeImageText = large_image_text;

  /* print new values */
  char* client_id = get_ini_value(p, "Identifiers", "ClientID");
  print_config_values(&discordPresence, client_id);
  /* actaully update the presence */
  Discord_UpdatePresence(&discordPresence);
}

/* manually update discord connection */
void refresh_discord() {
  Discord_UpdateConnection();
  Discord_RunCallbacks();
}

/* initialize discord rich presence */
void init_discord(char* client_id) {
  DiscordEventHandlers handlers;
  memset(&handlers, 0, sizeof(handlers));
  handlers.ready = handleDiscordReady;
  handlers.errored = handleDiscordError;
  handlers.disconnected = handleDiscordDisconnected;
  if (strlen(client_id) < 1 || strcmp(client_id, "1234567890123456789") == 0) {
    printf(
        "ClientID not correct (or not set).\n");
    shutdown_discord(1);
  }
  Discord_Initialize(client_id, &handlers, 1, NULL);
}