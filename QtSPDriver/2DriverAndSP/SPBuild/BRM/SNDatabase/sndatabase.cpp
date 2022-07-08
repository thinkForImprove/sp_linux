#include <fstream>
#include <sstream>
#include <string.h>
#include "sqlite3.h"
#include "sndatabase.h"

SNDatabase::SNDatabase() :
    m_db(nullptr)
{
}
SNDatabase::SNDatabase(std::string db_path) :
    m_db(nullptr)
{
    init_db(db_path);
}
SNDatabase::~SNDatabase()
{
    close_db();
}


int SNDatabase::exed_callback(void* param, int count, char** values, char** keys)
{
    Q_UNUSED(param);
    Q_UNUSED(count);
    Q_UNUSED(values);
    Q_UNUSED(keys);

    return 0;
}

void SNDatabase::close_db()
{
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

bool SNDatabase::init_db(std::string db_path, bool recreate/* = false*/)
{
    int ret = SQLITE_OK;
    const char* sql = nullptr;
    char* err_msg = nullptr;

    if (db_path.empty()) {
        return false;
    }

    close_db();

    // 检查数据库文件是否存在
    std::fstream fs(db_path);
    if (fs.is_open()) {
        fs.close();
        if (recreate == false) {
            // 创建数据库
            ret = sqlite3_open(db_path.c_str(), &m_db);
            if (ret != SQLITE_OK) {
                printf("sqlite3_open failed: %s\n", sqlite3_errmsg(m_db));
                return false;
            }
            return true;
        }
        remove(db_path.c_str());
    }

    // 创建数据库
    ret = sqlite3_open(db_path.c_str(), &m_db);
    if (ret != SQLITE_OK) {
        printf("sqlite3_open failed: %s\n", sqlite3_errmsg(m_db));
        return false;
    }

    // 创建动作信息表
    sql =   "CREATE TABLE ActInfo_Table ("
            "ActID TEXT NOT NULL,"
            "ActName TEXT NOT NULL,"
            "ActTime DATETIME NOT NULL,"
            "NumOfNotes INTEGER NOT NULL);";
    ret = sqlite3_exec(m_db, sql, SNDatabase::exed_callback, nullptr, &err_msg);
    if (ret != SQLITE_OK) {
        printf("sqlite3_exec for ActInfo_Table failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        close_db();
        return false;
    }

    // 创建冠字号信息表
    sql =   "CREATE TABLE NoteInfo_Table ("
            "SerialNumber TEXT NOT NULL,"
            "CurrencyID TEXT NOT NULL,"
            "NoteID INTEGER NOT NULL,"
            "Value INTEGER NOT NULL,"
            "NoteVer TEXT NOT NULL,"
            "Origin TEXT NOT NULL,"
            "Destination TEXT NOT NULL,"
            "Category TEXT NOT NULL,"
            "RejectCause TEXT NOT NULL,"
            "SNImagePath TEXT NOT NULL,"
            "FullImagePath1 TEXT,"
            "FullImagePath2 TEXT,"
            "ActID TEXT NOT NULL);";
    ret = sqlite3_exec(m_db, sql, SNDatabase::exed_callback, nullptr, &err_msg);
    if (ret != SQLITE_OK) {
        printf("sqlite3_exec for NoteInfo_Table failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        close_db();
        return false;
    }

    return true;
}

bool SNDatabase::save_sndb_info(SNDBInfo& sndb_info)
{
    bool b_ret = false;
    int ret = SQLITE_OK;
    char act_id[64];
    char act_time[64];
    char* err_msg = nullptr;
    size_t idx = 0;

    if (m_db == nullptr) {
        return false;
    }

    memset(act_id, 0x00, sizeof(act_id));
    sprintf(act_id, "%s%04d%02d%02d%02d%02d%02d%03d", sndb_info.act_name.c_str(),
            sndb_info.act_time.year, sndb_info.act_time.month, sndb_info.act_time.day,
            sndb_info.act_time.hour, sndb_info.act_time.minute, sndb_info.act_time.second, sndb_info.act_time.milli_second);
    memset(act_time, 0x00, sizeof(act_time));
    sprintf(act_time, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            sndb_info.act_time.year, sndb_info.act_time.month, sndb_info.act_time.day,
            sndb_info.act_time.hour, sndb_info.act_time.minute, sndb_info.act_time.second, sndb_info.act_time.milli_second);

    ret = sqlite3_exec(m_db, "BEGIN;", exed_callback, nullptr, &err_msg);
    if (ret != SQLITE_OK) {
        printf("sqlite3_exec(BEGIN) for save_sndb_info failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    b_ret = insert_into_ActInfo_Table(act_id, sndb_info.act_name, act_time, sndb_info.num_of_notes);
    if (b_ret == false) {
        printf("insert_into_ActInfo_Table for save_sndb_info failed\n");
        return false;
    }

    for (idx = 0; idx < sndb_info.notes_info.size(); idx++) {
        b_ret = insert_into_NoteInfo_Table(act_id,
                                           sndb_info.notes_info[idx].serial_number,
                                           sndb_info.notes_info[idx].currency_id,
                                           sndb_info.notes_info[idx].note_id,
                                           sndb_info.notes_info[idx].value,
                                           sndb_info.notes_info[idx].note_ver,
                                           sndb_info.notes_info[idx].origin,
                                           sndb_info.notes_info[idx].destination,
                                           sndb_info.notes_info[idx].category,
                                           sndb_info.notes_info[idx].reject_cause,
                                           sndb_info.notes_info[idx].sn_image_path,
                                           sndb_info.notes_info[idx].full_image_path1,
                                           sndb_info.notes_info[idx].full_image_path2);
        if (b_ret == false) {
            printf("insert_into_NoteInfo_Table for save_sndb_info failed\n");
            sqlite3_exec(m_db, "ROLLBACK;", exed_callback, nullptr, &err_msg);
            return false;
        }
    }

    ret = sqlite3_exec(m_db, "COMMIT;", exed_callback, nullptr, &err_msg);
    if (ret != SQLITE_OK) {
        printf("sqlite3_exec(COMMIT) for save_sndb_info failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool SNDatabase::delete_sndb_info(SNDBTimeInfo& deadline_time)
{
    int ret = SQLITE_OK;
    char act_time[64];
    char* err_msg = nullptr;
    std::ostringstream sql;

    if (m_db == nullptr) {
        return false;
    }

    memset(act_time, 0x00, sizeof(act_time));
    sprintf(act_time, "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            deadline_time.year, deadline_time.month, deadline_time.day,
            deadline_time.hour, deadline_time.minute, deadline_time.second, deadline_time.milli_second);

    ret = sqlite3_exec(m_db, "BEGIN;", exed_callback, nullptr, &err_msg);
    if (ret != SQLITE_OK) {
        printf("sqlite3_exec(BEGIN) for delete_sndb_info failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    sql <<  "DELETE FROM NoteInfo_Table WHERE ActID IN ("
            "SELECT ActID FROM ActInfo_Table WHERE ActTime < "
            "strftime('%Y-%m-%d %H:%M:%f', '" << act_time << "'));";
    ret = sqlite3_exec(m_db, sql.str().c_str(), exed_callback, nullptr, &err_msg);
    if (ret != SQLITE_OK) {
        printf("sqlite3_exec for DELETE FROM NoteInfo_Table failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    sql.clear();
    sql << "DELETE FROM ActInfo_Table WHERE ActTime < "
           "strftime('%Y-%m-%d %H:%M:%f', '" << act_time << "');";
    ret = sqlite3_exec(m_db, sql.str().c_str(), exed_callback, nullptr, &err_msg);
    if (ret != SQLITE_OK) {
        printf("sqlite3_exec for DELETE FROM ActInfo_Table failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(m_db, "ROLLBACK;", exed_callback, nullptr, &err_msg);
        return false;
    }

    ret = sqlite3_exec(m_db, "COMMIT;", exed_callback, nullptr, &err_msg);
    if (ret != SQLITE_OK) {
        printf("sqlite3_exec(COMMIT) for delete_sndb_info failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool SNDatabase::insert_into_ActInfo_Table(
        std::string act_id, std::string act_name, std::string act_time, int num_of_notes)
{
    std::ostringstream sql;
    char* err_msg = nullptr;
    int ret = SQLITE_OK;

    if (m_db == nullptr) {
        return false;
    }

    sql <<  "INSERT INTO ActInfo_Table ("
            "ActID, ActName, ActTime, NumOfNotes) "
            "VALUES ('" <<
            act_id << "', '" <<
            act_name << "', " <<
            "strftime('%Y-%m-%d %H:%M:%f', '" << act_time << "'), " <<
            num_of_notes << ");";
    ret = sqlite3_exec(m_db, sql.str().c_str(), SNDatabase::exed_callback, nullptr, &err_msg);
    if (ret != SQLITE_OK) {
        printf("sqlite3_exec for insert_into_ActInfo_Table failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}

bool SNDatabase::insert_into_NoteInfo_Table(
        std::string act_id, std::string serial_number, std::string currency_id, int note_id, int value, std::string note_ver,
        std::string origin, std::string destination, std::string category, std::string reject_cause,
        std::string sn_image_path, std::string full_image_path1/* = ""*/, std::string full_image_path2/* = ""*/)
{
    std::ostringstream sql;
    char* err_msg = nullptr;
    int ret = SQLITE_OK;

    if (m_db == nullptr) {
        return false;
    }

    sql <<  "INSERT INTO NoteInfo_Table ("
            "SerialNumber, CurrencyID, NoteID, Value, NoteVer, Origin, Destination, Category,"
            "RejectCause, SNImagePath, FullImagePath1, FullImagePath2, ActID) "
            "VALUES ('" <<
            serial_number << "', '" <<
            currency_id << "', " <<
            note_id << ", " <<
            value << ", '" <<
            note_ver << "', '" <<
            origin << "', '" <<
            destination << "', '" <<
            category << "', '" <<
            reject_cause << "', '" <<
            sn_image_path << "', '" <<
            full_image_path1 << "', '" <<
            full_image_path2 << "', '" <<
            act_id << "');";
    ret = sqlite3_exec(m_db, sql.str().c_str(), SNDatabase::exed_callback, nullptr, &err_msg);
    if (ret != SQLITE_OK) {
        printf("sqlite3_exec for insert_into_NoteInfo_Table failed: %s\n", err_msg);
        sqlite3_free(err_msg);
        return false;
    }

    return true;
}
