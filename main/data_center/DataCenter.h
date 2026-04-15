#pragma once

#include "Account.h"

class DataCenter {
public:
    const char* _name;
    Account _accountMain;

public:
    DataCenter(const char* name);
    ~DataCenter();

    bool addAccount(Account* account);
    bool removeAccount(Account* account);
    bool remove(Account::AccountVector* vec, Account* account);
    
    Account* searchAccount(const char* id);
    Account* find(Account::AccountVector* vec, const char* id);
    size_t getAccountLen() const;

private:
    Account::AccountVector _accountPool;
};
