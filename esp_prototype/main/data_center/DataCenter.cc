#include "DataCenter.h"
#include "DataCenterLog.h"
#include <cstring>
#include <algorithm>

#define DC_USE_AUTO_CLOSE 0

DataCenter::DataCenter(const char* name) : _accountMain(name, this) {
    _name = name;
}

DataCenter::~DataCenter() {
#if DC_USE_AUTO_CLOSE
    DC_LOG_INFO("DataCenter[%s] closing...", _name);
    while (!_accountPool.empty()) {
        Account* account = _accountPool.back();

        DC_LOG_INFO("Delete: %s", account->_id);
        delete account;

        _accountPool.pop_back();
    }
    DC_LOG_INFO("DataCenter[%s] closed.", _name);
#endif
}

Account* DataCenter::searchAccount(const char* id) {
    return find(&_accountPool, id);
}

Account* DataCenter::find(Account::AccountVector* vec, const char* id) {
    for (auto iter : *vec) {
        if (std::strcmp(id, iter->_id) == 0) {
            return iter;
        }
    }
    return nullptr;
}

bool DataCenter::addAccount(Account* account) {
    if (account == &_accountMain) {
        return false;
    }

    if (searchAccount(account->_id) != nullptr) {
        DC_LOG_ERROR("Multi add Account[%s]", account->_id);
        return false;
    }

    _accountPool.push_back(account);

    _accountMain.subscribe(account->_id);

    return true;
}

bool DataCenter::removeAccount(Account* account) {
    return remove(&_accountPool, account);
}

bool DataCenter::remove(Account::AccountVector* vec, Account* account) {
    auto iter = std::find(vec->begin(), vec->end(), account);

    if (iter == vec->end()) {
        DC_LOG_ERROR("Account[%s] was not found", account->_id);
        return false;
    }

    vec->erase(iter);

    return true;
}

size_t DataCenter::getAccountLen() const {
    return _accountPool.size();
}
