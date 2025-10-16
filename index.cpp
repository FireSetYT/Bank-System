#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <windows.h>
#include <limits>

using namespace std;
void clearConsole();
void waitForReturnToMenu();

enum AccountType { DEPOSIT, CREDIT };

string accountTypeToString(AccountType type) {
    return (type == DEPOSIT) ? "Депозит" : "Кредитний";
}

struct BankAccount {
    string accountNumber;
    AccountType type;
    double balance;
    bool isPrimary;
};

struct Transaction {
    string fromAccount;
    string toAccount;
    double amount;
    double fee;
};

struct Client {
    string name;
    string login;
    string password;
    vector<BankAccount> accounts;
    vector<Transaction> history;

    void addAccount(const BankAccount& acc) {
        if (acc.isPrimary) {
            for (auto& a : accounts) a.isPrimary = false;
        }
        accounts.push_back(acc);
    }

    BankAccount* findAccount(const string& number) {
        for (auto& acc : accounts) {
            if (acc.accountNumber == number)
                return &acc;
        }
        return nullptr;
    }

    BankAccount* getPrimaryAccount() {
        for (auto& acc : accounts) {
            if (acc.isPrimary)
                return &acc;
        }
        return nullptr;
    }

    void printHistory() {
        clearConsole();
        cout << "\n Історія транзакцій:\n";
        for (const auto& t : history) {
            cout << "З: " << t.fromAccount << "  На: " << t.toAccount
                 << " | Сума: " << fixed << setprecision(2) << t.amount
                 << " | Комісія: " << t.fee << " грн\n";
        }
        if (history.empty()) {
            cout << "Немає транзакцій.\n";
        }
        waitForReturnToMenu();
    }
};


void sortAccountsByBalance(Client& c);
void filterAccountsByType(const Client& c);
void filterAccountsByOwner(const vector<Client>& clients);


void clearConsole() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void waitForReturnToMenu() {
    char key;
    cout << "\n Бажаєте повернутись до меню? (y/n): ";
    while (true) {
        cin >> key;
        key = tolower(key);

        if (key == 'y') {
            system("cls");
            break;
        } else if (key == 'n') {
            cout << " Вихід скасовано. Залишаємося на поточному екрані.\n";
            break;
        } else {
            cout << " Некоректне введення. Введіть 'y' (так) або 'n' (ні): ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }
}

void saveClientsToFile(const vector<Client>& clients) {
    ofstream file("clients.txt");
    for (const auto& c : clients) {
        file << c.login << "," << c.password << "," << c.name << "\n";
        for (const auto& acc : c.accounts) {
            file << acc.accountNumber << "," << acc.type << "," << acc.balance << "," << acc.isPrimary << "\n";
        }
        file << "----\n";
    }
    file.close();
}

void saveTransactions(const Client& c) {
    ofstream file("transactions_" + c.login + ".txt", ios::app);
    for (const auto& t : c.history) {
        file << t.fromAccount << "," << t.toAccount << "," << t.amount << "," << t.fee << "\n";
    }
    file.close();
}

void registerClient(vector<Client>& clients) {
    clearConsole();
    Client c;
    cout << "\nРеєстрація нового клієнта\n";
    cout << "Ім'я: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, c.name);
    cout << "Логін: ";
    getline(cin, c.login);
    cout << "Пароль: ";
    getline(cin, c.password);

    for (const auto& cl : clients) {
        if (cl.login == c.login) {
            cout << " Такий логін вже існує.\n";
            return;
        }
    }

    clients.push_back(c);
    cout << " Клієнта зареєстровано!\n";
    saveClientsToFile(clients);
    waitForReturnToMenu();
}

Client* loginClient(vector<Client>& clients) {
    clearConsole();
    string login, password;
    cout << "\n Вхід у систему\n";
    cout << "Логін: ";
    cin >> login;
    cout << "Пароль: ";
    cin >> password;

    for (auto& c : clients) {
        if (c.login == login && c.password == password) {
            cout << " Вхід успішний! Вітаємо, " << c.name << "!\n";
            return &c;
        }
    }
    cout << " Невірні дані.\n";
    return nullptr;
}

void addAccount(Client& c) {
    clearConsole();
    BankAccount acc;
    int typeInput;

    cout << "\n Додати рахунок\n";
    cout << "Номер рахунку: ";
    cin >> acc.accountNumber;
    cout << "Тип (0 - депозит, 1 - кредит): ";
    cin >> typeInput;
    acc.type = static_cast<AccountType>(typeInput);
    cout << "Початковий баланс: ";
    cin >> acc.balance;
    cout << "Зробити основним? (1 - так, 0 - ні): ";
    cin >> acc.isPrimary;

    c.addAccount(acc);
    cout << " Рахунок додано!\n";
    waitForReturnToMenu();
}

void showAccounts(const Client& c) {
    clearConsole();
    cout << "\n Ваші рахунки:\n";
    for (const auto& acc : c.accounts) {
        cout << "- Номер: " << acc.accountNumber
             << " | Тип: " << accountTypeToString(acc.type)
             << " | Баланс: " << fixed << setprecision(2) << acc.balance
             << " грн | Основний: " << (acc.isPrimary ? "Так" : "Ні") << endl;
    }
    if (c.accounts.empty())
        cout << "У вас немає рахунків.\n";
    waitForReturnToMenu();
}

void transfer(Client& sender, vector<Client>& clients) {
    clearConsole();
    string fromAcc, toAcc;
    double amount;
    const double feeRate = 0.01;

    cout << "\n Переказ коштів\n";
    cout << "З рахунку (номер): ";
    cin >> fromAcc;
    cout << "На рахунок (номер): ";
    cin >> toAcc;
    cout << "Сума переказу: ";
    cin >> amount;

    if (amount <= 0) {
        cout << " Сума має бути більшою за 0.\n";
        waitForReturnToMenu();
        return;
    }

    BankAccount* from = sender.findAccount(fromAcc);
    BankAccount* to = nullptr;
    Client* recipient = nullptr;

    if (!from) {
        cout << " Відправник не знайдено.\n";
        waitForReturnToMenu();
        return;
    }

    // Перевірка на власні рахунки
    to = sender.findAccount(toAcc);
    if (to && from != to) {
        recipient = &sender;
    } else {
        // Пошук в інших клієнтах
        for (auto& c : clients) {
            if (&c == &sender) continue;
            to = c.findAccount(toAcc);
            if (to) {
                recipient = &c;
                break;
            }
        }
    }

    if (!to || !recipient) {
        cout << " Отримувача не знайдено.\n";
        waitForReturnToMenu();
        return;
    }

    double fee = amount * feeRate;
    double total = amount + fee;

    if (from->balance < total) {
        cout << " Недостатньо коштів (включаючи комісію).\n";
        waitForReturnToMenu();
        return;
    }

    from->balance -= total;
    to->balance += amount;

    Transaction t = {from->accountNumber, to->accountNumber, amount, fee};
    sender.history.push_back(t);
    recipient->history.push_back(t);

    saveTransactions(sender);
    if (recipient != &sender) {
        saveTransactions(*recipient);
    }

    cout << " Переказ успішний. Комісія: " << fee << " грн\n";
    waitForReturnToMenu();
}


void sortAccountsByBalance(Client& c) {
    clearConsole();
    sort(c.accounts.begin(), c.accounts.end(), [](const BankAccount& a, const BankAccount& b) {
        return a.balance > b.balance;
    });
    cout << " Рахунки відсортовано за балансом (спадання):\n";
    showAccounts(c);
    waitForReturnToMenu();
}

void filterAccountsByType(const Client& c) {
    clearConsole();
    int typeInput;
    cout << "Введіть тип рахунку для фільтрації (0 - депозит, 1 - кредит): ";
    cin >> typeInput;

    AccountType type = static_cast<AccountType>(typeInput);
    cout << "\n Результати фільтрації:\n";

    bool found = false;
    for (const auto& acc : c.accounts) {
        if (acc.type == type) {
            cout << "- Номер: " << acc.accountNumber
                 << " | Баланс: " << acc.balance
                 << " | Основний: " << (acc.isPrimary ? "Так" : "Ні") << endl;
            found = true;
        }
    }

    if (!found) cout << " Немає рахунків такого типу.\n";
    waitForReturnToMenu();
}

void filterAccountsByOwner(const vector<Client>& clients) {
    clearConsole();
    string ownerName;
    cout << "Введіть ім'я власника для пошуку: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, ownerName);

    cout << "\n Результати пошуку за власником \"" << ownerName << "\":\n";
    bool found = false;

    for (const auto& c : clients) {
        if (c.name == ownerName) {
            for (const auto& acc : c.accounts) {
                cout << "- Рахунок: " << acc.accountNumber
                     << " | Баланс: " << acc.balance
                     << " | Тип: " << accountTypeToString(acc.type)
                     << " | Основний: " << (acc.isPrimary ? "Так" : "Ні") << endl;
                found = true;
            }
        }
    }

    if (!found) cout << " Клієнта або рахунків не знайдено.\n";
    waitForReturnToMenu();
}

void userMenu(Client& currentClient, vector<Client>& clients) {
    int option;
    do {
        clearConsole();
        cout << "\n=====  МЕНЮ КЛІЄНТА =====\n";
        cout << "1.  Додати рахунок\n";
        cout << "2.  Переглянути рахунки\n";
        cout << "3.  Переказати кошти\n";
        cout << "4.  Переглянути історію транзакцій\n";
        cout << "5.  Сортувати рахунки за балансом\n";
        cout << "6.  Фільтрація рахунків за типом\n";
        cout << "7.  Пошук рахунків за власником\n";
        cout << "0.  Вийти\n";
        cout << "Оберіть опцію: ";
        cin >> option;

        switch (option) {
            case 1: addAccount(currentClient); break;
            case 2: showAccounts(currentClient); break;
            case 3: transfer(currentClient, clients); break;
            case 4: currentClient.printHistory(); break;
            case 5: sortAccountsByBalance(currentClient); break;
            case 6: filterAccountsByType(currentClient); break;
            case 7: filterAccountsByOwner(clients); break;
            case 0: cout << "Повернення до головного меню\n"; break;
            default: cout << "Невірна опція\n";
        }
    } while (option != 0);
}


int main() {
    SetConsoleOutputCP(CP_UTF8);
    setlocale(LC_ALL, "uk_UA.UTF-8");

    vector<Client> clients;
    int option;

    do {
        clearConsole();
        cout << "\n=====  ГОЛОВНЕ МЕНЮ =====\n";
        cout << "1. Зареєструватися\n";
        cout << "2. Увійти\n";
        cout << "0. Вийти\n";
        cout << "Оберіть опцію: ";
        cin >> option;

        switch (option) {
            case 1: registerClient(clients); break;
            case 2: {
                Client* current = loginClient(clients);
                if (current)
                    userMenu(*current, clients);
                break;
            }
            case 0: cout << " До побачення!\n"; break;
            default: cout << " Невірна опція\n";
        }
    } while (option != 0);

    saveClientsToFile(clients);
    return 0;
}