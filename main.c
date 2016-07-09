/**
 * @brief Testbed for elektra internalnotification plugin
 *
 * @copyright BSD 3-Clause License (see LICENSE)
 *
 */

#include <kdb.h>
#include <kdbhelper.h>

#include <unistd.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// from http://doc.libelektra.org/api/latest/html/group__kdb.html
int printKeyWarnings(Key *warningKey) {
  const Key *metaWarnings = keyGetMeta(warningKey, "warnings");
  if (!metaWarnings)
    return 1; /* There are no current warnings */
  int nrWarnings = atoi(keyString(metaWarnings));
  char buffer[] = "warnings/#00\0description";
  printf("There are %d warnings\n", nrWarnings + 1);
  for (int i = 0; i <= nrWarnings; ++i) {
    buffer[10] = i / 10 % 10 + '0';
    buffer[11] = i % 10 + '0';
    printf("buffer is: %s\n", buffer);
    strncat(buffer, "/number", sizeof(buffer) - 1);
    printf("number: %s\n", keyString(keyGetMeta(warningKey, buffer)));
    buffer[12] = '\0';
    strncat(buffer, "/description", sizeof(buffer) - 1);
    printf("description: %s\n", keyString(keyGetMeta(warningKey, buffer)));
    buffer[12] = '\0';
    strncat(buffer, "/ingroup", sizeof(buffer) - 1);
    keyGetMeta(warningKey, buffer);
    printf("ingroup: %s\n", keyString(keyGetMeta(warningKey, buffer)));
    buffer[12] = '\0';
    strncat(buffer, "/module", sizeof(buffer) - 1);
    keyGetMeta(warningKey, buffer);
    printf("module: %s\n", keyString(keyGetMeta(warningKey, buffer)));
    buffer[12] = '\0';
    strncat(buffer, "/file", sizeof(buffer) - 1);
    keyGetMeta(warningKey, buffer);
    printf("file: %s\n", keyString(keyGetMeta(warningKey, buffer)));
    buffer[12] = '\0';
    strncat(buffer, "/line", sizeof(buffer) - 1);
    keyGetMeta(warningKey, buffer);
    printf("line: %s\n", keyString(keyGetMeta(warningKey, buffer)));
    buffer[12] = '\0';
    strncat(buffer, "/reason", sizeof(buffer) - 1);
    keyGetMeta(warningKey, buffer);
    printf("reason: %s\n", keyString(keyGetMeta(warningKey, buffer)));
    buffer[12] = '\0';
    strncat(buffer, "/mountpoint", sizeof(buffer) - 1);
    keyGetMeta(warningKey, buffer);
    printf("reason: %s\n", keyString(keyGetMeta(warningKey, buffer)));
    buffer[12] = '\0';
    strncat(buffer, "/configfile", sizeof(buffer) - 1);
    keyGetMeta(warningKey, buffer);
    printf("reason: %s\n", keyString(keyGetMeta(warningKey, buffer)));
  }
  return 0;
}

// from http://doc.libelektra.org/api/latest/html/group__kdb.html
int printKeyErrors(Key *errorKey) {
  const Key *metaError = keyGetMeta(errorKey, "error");
  if (!metaError)
    return 1; /* There is no current error */
  printf("number: %s\n", keyString(keyGetMeta(errorKey, "error/number")));
  printf("description: : %s\n",
         keyString(keyGetMeta(errorKey, "error/description")));
  printf("ingroup: : %s\n", keyString(keyGetMeta(errorKey, "error/ingroup")));
  printf("module: : %s\n", keyString(keyGetMeta(errorKey, "error/module")));
  printf("at: %s:%s\n", keyString(keyGetMeta(errorKey, "error/file")),
         keyString(keyGetMeta(errorKey, "error/line")));
  printf("reason: : %s\n", keyString(keyGetMeta(errorKey, "error/reason")));
  printf("mountpoint: : %s\n",
         keyString(keyGetMeta(errorKey, "error/mountpoint")));
  printf("configfile: : %s\n",
         keyString(keyGetMeta(errorKey, "error/configfile")));
  return 0;
}

int internalnotificationRegisterInt(KDB *kdb, int *variable, Key *key) {
  typedef int (*elektraInternalnotificationRegisterIntCallback)(
      void *handle, int *variable, Key *key);

  static size_t address = 0;
  static size_t handle;

  if (address == 0) {
    char *NOTIFICATION_BASE = "system/elektra/modules/internalnotification";
    char *EXPORTED_FUNCTION = "system/elektra/modules/internalnotification/"
                              "exports/elektraInternalnotificationRegisterInt";
    char *EXPORTED_HANDLE =
        "system/elektra/modules/internalnotification/exports/handle";
    Key *parentKey = keyNew(NOTIFICATION_BASE, KEY_END);

    KeySet *conf = ksNew(20, KS_END);
    kdbGet(kdb, conf, parentKey);
    Key *keyFunction = ksLookupByName(conf, EXPORTED_FUNCTION, 0);

    if (keyFunction == 0 || !keyIsBinary(keyFunction)) {
      // Key value is not binary
      return -1;
    }

    size_t *buffer;
    size_t bufferSize = keyGetValueSize(keyFunction);
    buffer = (size_t *)elektraMalloc(bufferSize);
    if (buffer == NULL) {
      // Malloc failed
      return -1;
    }
    if (keyGetBinary(keyFunction, buffer, bufferSize) == -1) {
      return -1;
    }

    // verify that address is not null
    if (buffer == NULL) {
      return -1;
    }

    // convert address from buffer
    address = *buffer;

    // free buffer
    elektraFree(buffer);

    Key *keyHandle = ksLookupByName(conf, EXPORTED_HANDLE, 0);

    if (keyHandle == 0 || !keyIsBinary(keyHandle)) {
      // Key value is not binary
      return -1;
    }

    size_t *handleBuffer;
    size_t handleBufferSize = keyGetValueSize(keyHandle);
    handleBuffer = (size_t *)elektraMalloc(handleBufferSize);
    if (handleBuffer == NULL) {
      // Malloc failed
      return -1;
    }
    if (keyGetBinary(keyHandle, handleBuffer, handleBufferSize) == -1) {
      return -1;
    }

    // verify that handle is not null
    if (handleBuffer == NULL) {
      return -1;
    }

    // convert address from buffer
    handle = *handleBuffer;

    // free buffer
    elektraFree(handleBuffer);
  }

  // Call function
  return ((elektraInternalnotificationRegisterIntCallback)address)(
      (void *)handle, variable, key);
}

void updateAndPrintGreeting(KDB *kdb, KeySet *ks, Key *parentKey) {
  int result = kdbGet(kdb, ks, parentKey);
  printf("kdbGet result was %d\n", result);
  printKeyWarnings(parentKey);
  printKeyErrors(parentKey);

  Key *k = ksLookupByName(ks, "/sw/waht/elektra-test/greeting", 0);
  if (k != 0) {
    printf("Current \"greeting\" is: %s\n", keyString(k));
  } else {
    printf("Key \"%s\" not found!\n", "/sw/waht/elektra-test/greeting");
  }
}

void setTemperatureToRandomValue(KDB *kdb, KeySet *ks, Key *parentKey) {
  Key *k = ksLookupByName(ks, "/sw/waht/elektra-test/temperature", 0);
  if (k == 0) {
    k = keyNew("user/sw/waht/elektra-test/temperature", KEY_END);
    ksAppendKey(ks, k);
  }

  int value = rand() / 2500;

  printf("random value = %d\n", value);

  int bufferSize = 250;
  char buffer[250];
  snprintf(buffer, bufferSize, "%d", value);
  keySetString(k, buffer);

  int result = kdbSet(kdb, ks, parentKey);
  printf("kdbSet result was %d\n", result);
  printKeyWarnings(parentKey);
  printKeyErrors(parentKey);
}

int main(int argc, char **argv) {
  srand(time(NULL));

  printf("internalnotification testbed\n");
  printf("Press t to print \"temperature\"\n");
  printf("Press g to kdbGet and print \"greeting\" key\n");
  printf("Press s to set \"temperature\" key to a random value\n\n");

  Key *parentKey = keyNew("/sw/waht/elektra-test", KEY_END);
  Key *keyTemp = keyNew("/sw/waht/elektra-test/temperature", KEY_END);

  KDB *kdb = kdbOpen(parentKey);
  KeySet *ks = ksNew(10, KS_END);

  int temperature = 0;
  int result = internalnotificationRegisterInt(kdb, &temperature, keyTemp);
  if (result != 1) {
    fprintf(
        stderr,
        "Could not register variable with internalnotification plugin: %d!\n",
        result);
    fprintf(stderr,
            "Make sure the \"internalnotification\" plugin is loaded.\n");
    exit(-1);
  }
  printf("temperature variable located at %p\n", &temperature);

  // get data intially
  kdbGet(kdb, ks, parentKey);

  while (1) {
    char ch;
    do {
      ch = getchar();
    } while (isspace(ch));

    switch (ch) {
    case 's':
      setTemperatureToRandomValue(kdb, ks, parentKey);
      break;
    case 'g':
      updateAndPrintGreeting(kdb, ks, parentKey);
      break;
    case 't':
      printf("\"temperature\" value = %d\n", temperature);
      break;
    case 'q':
      kdbClose(kdb, parentKey);
      exit(0);
    default:
      printf("Unknown key pressed!\n\n");
      break;
    }
  }

  return -1;
}
