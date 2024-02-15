#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "lmdb.h"

#define DATABASE_SIZE 10485760

// {
//     "ProtocolVersion": 6.0,
//     "IsVM": "No",
//     "LicensingStatus": 2,
//     "RemainingTime": "43200 minutes",
//     "ApplicationID": "0ff1ce15-a989-479d-af46-f275c6370663 (Office2013+)",
//     "SKUID": "76881159-155c-43e0-9db7-2d70a9a3a4ca (Office Project Pro 2021)",
//     "KMSID": "86d50b16-4808-41af-b83b-b338274318b2 (Office 2021)",
//     "ClientMachineID": "a3492128-b441-4645-8468-b90306b50e22",
//     "PreviousClientMachineID": "00000000-0000-0000-0000-000000000000",
//     "ClientRequestTimestamp": "2024-01-31 16:53:12",
//     "ClientIP": "[2001:861:560c:ed95:45ab:3701:8104:4a5f]:62405",
//     "WorkstationName": "mx1.sony.net",
//     "NCountPolicy": 5
// }

// {"ProtocolVersion":6.0,"IsVM":"No","LicensingStatus":2,"RemainingTime":"43200 minutes","ApplicationID":"0ff1ce15-a989-479d-af46-f275c6370663 (Office2013+)","SKUID":"76881159-155c-43e0-9db7-2d70a9a3a4ca (Office Project Pro 2021)","KMSID":"86d50b16-4808-41af-b83b-b338274318b2 (Office 2021)","ClientMachineID":"a3492128-b441-4645-8468-b90306b50e22","PreviousClientMachineID":"00000000-0000-0000-0000-000000000000","ClientRequestTimestamp":"2024-01-31 16:53:12","ClientIP":"[2001:861:560c:ed95:45ab:3701:8104:4a5f]:62405","WorkstationName":"mx1.sony.net","NCountPolicy":5}

int main()
{
    MDB_env *env;
    MDB_dbi dbi;
    MDB_val key, data;
    MDB_txn *txn;
    int rc;

    // Créer un environnement de base de données
    rc = mdb_env_create(&env);
    if (rc != 0)
    {
        fprintf(stderr, "mdb_env_create failed, error %d\n", rc);
        return EXIT_FAILURE;
    }

    // Définir la taille de la base de données
    rc = mdb_env_set_mapsize(env, DATABASE_SIZE);
    if (rc != 0)
    {
        fprintf(stderr, "mdb_env_set_mapsize failed, error %d\n", rc);
        return EXIT_FAILURE;
    }

    // Ouvrir la base de données en mémoire
    rc = mdb_env_open(env, "tmpdb", MDB_NOSUBDIR, 0600);
    if (rc != 0)
    {
        fprintf(stderr, "mdb_env_open failed, error %d\n", rc);
        return EXIT_FAILURE;
    }

    // Commencer une transaction
    rc = mdb_txn_begin(env, NULL, 0, &txn);
    if (rc != 0)
    {
        fprintf(stderr, "mdb_txn_begin failed, error %d\n", rc);
        return EXIT_FAILURE;
    }

    // Ouvrir le handle de la base de données
    rc = mdb_dbi_open(txn, NULL, 0, &dbi);
    if (rc != 0)
    {
        fprintf(stderr, "mdb_dbi_open failed, error %d\n", rc);
        return EXIT_FAILURE;
    }

    char *json = "{\"ProtocolVersion\":6.0,\"IsVM\":\"No\",\"LicensingStatus\":2,\"RemainingTime\":\"43200 minutes\",\"ApplicationID\":\"0ff1ce15-a989-479d-af46-f275c6370663 (Office2013+)\",\"SKUID\":\"76881159-155c-43e0-9db7-2d70a9a3a4ca (Office Project Pro 2021)\",\"KMSID\":\"86d50b16-4808-41af-b83b-b338274318b2 (Office 2021)\",\"ClientMachineID\":\"a3492128-b441-4645-8468-b90306b50e22\",\"PreviousClientMachineID\":\"00000000-0000-0000-0000-000000000000\",\"ClientRequestTimestamp\":\"2024-01-31 16:53:12\",\"ClientIP\":\"[2001:861:560c:ed95:45ab:3701:8104:4a5f]:62405\",\"WorkstationName\":\"mx1.sony.net\",\"NCountPolicy\":%d}";

    for (int i = 0; i < 10000; i++)
    {
        char buffer[1000];
        char key_buffer[100];
        time_t now = time(NULL);
        printf("i: %d\n", i);
        sprintf(key_buffer, "log-%ld-%d", now, i);
        key.mv_size = strlen(key_buffer);
        key.mv_data = key_buffer;
        sprintf(buffer, json, i);
        data.mv_size = strlen(buffer);
        data.mv_data = buffer;
        rc = mdb_put(txn, dbi, &key, &data, 0);
        if (rc != 0)
        {
            if (rc == MDB_MAP_FULL)
            {
                fprintf(stderr, "mdb_put failed, error MDB_MAP_FULL\n");
                return EXIT_FAILURE;
            }
            return EXIT_FAILURE;
        }
    }

    // // Stocker la paire clé/valeur
    // rc = mdb_put(txn, dbi, &key, &data, 0);
    // if (rc != 0)
    // {
    //     fprintf(stderr, "mdb_put failed, error %d\n", rc);
    //     return EXIT_FAILURE;
    // }

    // Valider la transaction
    rc = mdb_txn_commit(txn);
    if (rc != 0)
    {
        fprintf(stderr, "mdb_txn_commit failed, error %d\n", rc);
        return EXIT_FAILURE;
    }
    // Déclarer une variable pour stocker la clé et la valeur
    MDB_val current_key, current_value;

    // Ouvrir une transaction en lecture seule
    rc = mdb_txn_begin(env, NULL, MDB_RDONLY, &txn);
    if (rc != 0)
    {
        fprintf(stderr, "mdb_txn_begin failed, error %d\n", rc);
        return EXIT_FAILURE;
    }

    // Ouvrir un curseur pour parcourir les clés et les valeurs
    MDB_cursor *cursor;
    rc = mdb_cursor_open(txn, dbi, &cursor);
    if (rc != 0)
    {
        fprintf(stderr, "mdb_cursor_open failed, error %d\n", rc);
        return EXIT_FAILURE;
    }

    // Parcourir toutes les clés et les valeurs
    while ((rc = mdb_cursor_get(cursor, &current_key, &current_value, MDB_NEXT)) == 0)
    {
        // Traiter la clé et la valeur ici
        // Vous pouvez accéder aux données via current_key.mv_data et current_value.mv_data

        // Afficher la clé et la valeur
        printf("Clé: %.*s\n", (int)current_key.mv_size, (char *)current_key.mv_data);
        printf("Valeur: %.*s\n", (int)current_value.mv_size, (char *)current_value.mv_data);
    }

    // Vérifier si une erreur s'est produite lors de la lecture des clés et des valeurs
    if (rc != MDB_NOTFOUND)
    {
        fprintf(stderr, "mdb_cursor_get failed, error %d\n", rc);
        return EXIT_FAILURE;
    }

    // Fermer le curseur
    mdb_cursor_close(cursor);

    // Valider et fermer la transaction
    rc = mdb_txn_commit(txn);
    if (rc != 0)
    {
        fprintf(stderr, "mdb_txn_commit failed, error %d\n", rc);
        return EXIT_FAILURE;
    }

    // Fermer la base de données
    mdb_dbi_close(env, dbi);
    mdb_env_close(env);

    return EXIT_SUCCESS;
}