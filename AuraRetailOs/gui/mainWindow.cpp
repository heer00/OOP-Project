#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <QFont>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QInputDialog>
#include "diagnosisdialog.h"


// include your builder
#include "core/KioskBuilder.h"
#include "factory/KioskFactorySimple.h"
#include "payment/WalletAdapter.h"
#include "payment/UserWallet.h"
#include "payment/UPIAdapter.h"
#include "payment/CardAdapter.h"
#include "inventory/Inventory.h"
#include "command/PurchaseItemCommand.h"
#include "command/RefundCommand.h"
#include "command/RestockCommand.h"
#include "persistence/TransactionLog.h"
#include "persistence/ConfigStore.h"
#include "hardware/SolarModule.h"
#include "hardware/NetworkModule.h"
#include "hardware/KioskModule.h"
#include <vector>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {

    ui->setupUi(this);

    // Load configuration to avoid stod parsing crashes
    ConfigStore config("data/config.json");
    config.load();

    userStore = new UserStore("data/users.json");
    userStore->load();

    kiosk = nullptr;
    
    // We are discarding the .ui layout to implement the new UI programmatically
    if (this->centralWidget() && this->centralWidget()->layout()) {
        QWidget* emptyWidget = new QWidget(this);
        this->setCentralWidget(emptyWidget);
    }
    
    setupProgrammaticUI();
    this->resize(900, 700);
}


MainWindow::~MainWindow() {
    delete ui;
    if (kiosk) {
        delete kiosk;
    }
    if (userStore) {
        userStore->save();
        delete userStore;
    }
}

void MainWindow::setupProgrammaticUI() {
    mainStack = new QStackedWidget(this);
    setCentralWidget(mainStack);

    QFont titleFont("Arial", 16, QFont::Bold);
    QFont logFont("Consolas", 10);
    logFont.setStyleHint(QFont::Monospace);

    // ==========================================
    // Page 0: Role Selection Menu
    // ==========================================
    roleMenuWidget = new QWidget();
    QVBoxLayout* roleLayout = new QVBoxLayout(roleMenuWidget);
    
    QLabel* titleLabel = new QLabel("Aura Kiosk System - Login");
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    QLabel* idLabel = new QLabel("Enter ID (Cust/Admin):");
    userIdInput = new QLineEdit();
    userIdInput->setPlaceholderText("Enter your ID here...");

    QLabel* typeLabel = new QLabel("Select Kiosk Type:");
    kioskTypeDropdown = new QComboBox();
    kioskTypeDropdown->addItems({"food", "pharmacy", "emergency"});

    btnCustomer = new QPushButton("Login as Customer");
    btnAdmin = new QPushButton("Login as Admin");

    roleLayout->addWidget(titleLabel);
    roleLayout->addSpacing(20);
    roleLayout->addWidget(typeLabel);
    roleLayout->addWidget(kioskTypeDropdown);
    roleLayout->addSpacing(10);
    roleLayout->addWidget(idLabel);
    roleLayout->addWidget(userIdInput);
    roleLayout->addSpacing(20);
    roleLayout->addWidget(btnCustomer);
    roleLayout->addWidget(btnAdmin);
    
    // Add Run Diagnosis to Login screen as well for better visibility
    QPushButton* btnMainDiagnosis = new QPushButton("Run System Diagnosis");
    btnMainDiagnosis->setStyleSheet("QPushButton { background-color: #333333; color: #4caf50; font-weight: bold; padding: 10px; }");
    roleLayout->addWidget(btnMainDiagnosis);
    connect(btnMainDiagnosis, &QPushButton::clicked, this, &MainWindow::onRunDiagnosisClicked);

    roleLayout->addStretch();


    // ==========================================
    // Page 1: Customer View
    // ==========================================
    customerWidget = new QWidget();
    QVBoxLayout* custLayout = new QVBoxLayout(customerWidget);

    QLabel* custTitle = new QLabel("Customer Portal");
    custTitle->setFont(titleFont);
    custTitle->setAlignment(Qt::AlignCenter);

    // Wallet section
    QHBoxLayout* walletLayout = new QHBoxLayout();
    walletBalanceLabel = new QLabel("Wallet Balance: Rs. 0.00");
    btnTopUpWallet = new QPushButton("Top Up Wallet");
    walletLayout->addWidget(walletBalanceLabel);
    walletLayout->addWidget(btnTopUpWallet);

    // Buy section
    QHBoxLayout* buyLayout = new QHBoxLayout();
    productDropdown = new QComboBox();
    
    // Populated dynamically on login
    
    buyQtyInput = new QSpinBox();
    buyQtyInput->setRange(1, 100);

    btnBuy = new QPushButton("Buy Item");
    buyLayout->addWidget(new QLabel("Select Product:"));
    buyLayout->addWidget(productDropdown);
    buyLayout->addWidget(new QLabel("Qty:"));
    buyLayout->addWidget(buyQtyInput);
    buyLayout->addWidget(btnBuy);

    // Refund section
    QHBoxLayout* refundLayout = new QHBoxLayout();
    transactionInput = new QLineEdit();
    transactionInput->setPlaceholderText("Txn ID");
    refundProductIdInput = new QLineEdit();
    refundProductIdInput->setPlaceholderText("Product ID");
    btnRefund = new QPushButton("Refund");
    refundLayout->addWidget(new QLabel("Refund:"));
    refundLayout->addWidget(transactionInput);
    refundLayout->addWidget(refundProductIdInput);
    refundLayout->addWidget(btnRefund);

    custLogBox = new QTextEdit();
    custLogBox->setReadOnly(true);
    custLogBox->setFont(logFont);
    custLogBox->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #d4d4d4; }");

    btnCustBack = new QPushButton("Logout (Back to Main Menu)");

    custLayout->addWidget(custTitle);
    custLayout->addLayout(walletLayout);
    custLayout->addLayout(buyLayout);
    custLayout->addLayout(refundLayout);
    custLayout->addWidget(custLogBox);
    custLayout->addWidget(btnCustBack);

    // ==========================================
    // Page 2: Admin View
    // ==========================================
    adminWidget = new QWidget();
    QVBoxLayout* adminLayout = new QVBoxLayout(adminWidget);

    QLabel* adminTitle = new QLabel("Admin Portal");
    adminTitle->setFont(titleFont);
    adminTitle->setAlignment(Qt::AlignCenter);

    // Restock section
    QHBoxLayout* restockLayout = new QHBoxLayout();
    restockProductIdInput = new QLineEdit();
    restockProductIdInput->setPlaceholderText("Product ID");
    restockQtyInput = new QSpinBox();
    restockQtyInput->setRange(1, 1000);
    btnRestock = new QPushButton("Restock Item");

    restockLayout->addWidget(new QLabel("Product ID:"));
    restockLayout->addWidget(restockProductIdInput);
    restockLayout->addWidget(new QLabel("Qty:"));
    restockLayout->addWidget(restockQtyInput);
    restockLayout->addWidget(btnRestock);

    adminLogBox = new QTextEdit();
    adminLogBox->setReadOnly(true);
    adminLogBox->setFont(logFont);
    adminLogBox->setStyleSheet("QTextEdit { background-color: #1e1e1e; color: #d4d4d4; }");

    btnAdminBack = new QPushButton("Logout (Back to Main Menu)");

    adminLayout->addWidget(adminTitle);
    adminLayout->addLayout(restockLayout);

    QHBoxLayout* adminActionsLayout = new QHBoxLayout();
    btnViewStock = new QPushButton("View Stock");
    btnViewTransactions = new QPushButton("View Transactions Log");
    adminActionsLayout->addWidget(btnViewStock);
    adminActionsLayout->addWidget(btnViewTransactions);
    adminLayout->addLayout(adminActionsLayout);

    QHBoxLayout* adminActionsLayout2 = new QHBoxLayout();
    btnRunDiagnosis = new QPushButton("Run Diagnosis");
    adminActionsLayout2->addWidget(btnRunDiagnosis);
    adminLayout->addLayout(adminActionsLayout2);

    // Simulation Section (New)
    adminLayout->addSpacing(10);
    adminLayout->addWidget(new QLabel("--- Simulation Controls ---"));
    QHBoxLayout* simLayout = new QHBoxLayout();
    batterySlider = new QSpinBox();
    batterySlider->setRange(0, 100);
    batterySlider->setValue(100);
    btnSetBattery = new QPushButton("Set Battery %");
    simLayout->addWidget(new QLabel("Simulate Battery:"));
    simLayout->addWidget(batterySlider);
    simLayout->addWidget(btnSetBattery);
    adminLayout->addLayout(simLayout);



    adminLayout->addWidget(adminLogBox);
    adminLayout->addWidget(btnAdminBack);

    // ==========================================
    // Add pages to StackedWidget
    // ==========================================
    mainStack->addWidget(roleMenuWidget); // Index 0
    mainStack->addWidget(customerWidget); // Index 1
    mainStack->addWidget(adminWidget);    // Index 2

    // Connect signals
    connect(btnCustomer, &QPushButton::clicked, this, &MainWindow::onRoleCustomerClicked);
    connect(btnAdmin, &QPushButton::clicked, this, &MainWindow::onRoleAdminClicked);
    connect(btnBuy, &QPushButton::clicked, this, &MainWindow::onBuyClicked);
    connect(btnRefund, &QPushButton::clicked, this, &MainWindow::onRefundClicked);
    connect(btnRestock, &QPushButton::clicked, this, &MainWindow::onRestockClicked);
    connect(btnViewStock, &QPushButton::clicked, this, &MainWindow::onViewStockClicked);
    connect(btnViewTransactions, &QPushButton::clicked, this, &MainWindow::onViewTransactionsClicked);
    connect(btnRunDiagnosis, &QPushButton::clicked, this, &MainWindow::onRunDiagnosisClicked);
    connect(btnTopUpWallet, &QPushButton::clicked, this, &MainWindow::onTopUpWalletClicked);

    connect(btnCustBack, &QPushButton::clicked, this, &MainWindow::onBackToMainClicked);
    connect(btnAdminBack, &QPushButton::clicked, this, &MainWindow::onBackToMainClicked);
    
    // New: Battery simulation
    connect(btnSetBattery, &QPushButton::clicked, this, [this](){
        KioskInterface* current = kiosk;
        while (current) {
            SolarModule* solar = dynamic_cast<SolarModule*>(current);
            if (solar) {
                solar->setBatteryLevel(batterySlider->value());
                adminLogBox->append(QString("<span style=\"color:#4caf50;\">[Sim] Battery set to %1%</span>").arg(batterySlider->value()));
                return;
            }
            KioskModule* mod = dynamic_cast<KioskModule*>(current);
            if (mod) current = mod->getWrappedKiosk();
            else break;
        }
        adminLogBox->append("<span style=\"color:#f44336;\">[Sim] SolarModule not found in this kiosk.</span>");
    });
}


void MainWindow::onRoleCustomerClicked() {
    currentUserId = userIdInput->text().trimmed();
    if (currentUserId.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter your Customer ID.");
        return;
    }

    if (!userStore->exists(currentUserId.toStdString())) {
        userStore->registerUser(currentUserId.toStdString(), "1234", 0.0);
        userStore->save();
    }
    
    // Dynamically build kiosk based on selection
    QString type = kioskTypeDropdown->currentText();
    if (!kiosk || type != lastKioskType) {
        if (kiosk) delete kiosk;
        KioskFactory* factory = KioskFactorySimple::createFactory(type.toStdString());
        baseKiosk = KioskBuilder()
            .addDispenser(factory->createDispenser())
            .addPayment(factory->createPayment())
            .addInventory(factory->createInventory())
            .addPricingPolicy(factory->createPricingPolicy())
            .build();
        
        // Wrap with decorators
        kiosk = new NetworkModule(new SolarModule(baseKiosk, 100), false);
        
        delete factory;
        lastKioskType = type;
    }

    
    // Refresh product list in case it changed
    productDropdown->clear();
    std::vector<std::string> products = getBaseKiosk()->getInventory()->getAllProductIds();
    for (const auto& p : products) {
        productDropdown->addItem(QString::fromStdString(p));
    }


    mainStack->setCurrentIndex(1); // Go to Customer Page
    custLogBox->append(QString("<span style=\"color:#888888;\">[%1]</span> Logged in as Customer: %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"), currentUserId));
    
    refreshWalletBalance();
}

void MainWindow::onRoleAdminClicked() {
    currentUserId = userIdInput->text().trimmed();
    if (currentUserId.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter your Admin ID.");
        return;
    }

    // Dynamically build kiosk based on selection
    QString type = kioskTypeDropdown->currentText();
    if (!kiosk || type != lastKioskType) {
        if (kiosk) delete kiosk;
        KioskFactory* factory = KioskFactorySimple::createFactory(type.toStdString());
        baseKiosk = KioskBuilder()
            .addDispenser(factory->createDispenser())
            .addPayment(factory->createPayment())
            .addInventory(factory->createInventory())
            .addPricingPolicy(factory->createPricingPolicy())
            .build();
        
        // Wrap with decorators
        kiosk = new NetworkModule(new SolarModule(baseKiosk, 100), false);

        delete factory;
        lastKioskType = type;
    }


    mainStack->setCurrentIndex(2); // Go to Admin Page
    adminLogBox->append(QString("<span style=\"color:#888888;\">[%1]</span> Logged in as Admin: %2")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss"), currentUserId));
}

void MainWindow::onBackToMainClicked() {
    currentUserId.clear();
    userIdInput->clear();
    mainStack->setCurrentIndex(0);
}

void MainWindow::onBuyClicked() {
    QString product = productDropdown->currentText();
    if (product.isEmpty()) return;
    int qty = buyQtyInput->value();

    // Ask for payment method
    QStringList paymentMethods;
    paymentMethods << "Wallet" << "UPI" << "Card";
    bool ok;
    QString method = QInputDialog::getItem(this, "Payment Method", 
                                           "Select payment method:", paymentMethods, 0, false, &ok);
    if (!ok || method.isEmpty()) return; // User cancelled

    if (method == "Wallet") {
        getBaseKiosk()->setPayment(new WalletAdapter(currentUserId.toStdString()));
    } else if (method == "UPI") {
        QString mobile = QInputDialog::getText(this, "UPI Payment", "Enter mobile number for UPI (10 digits):", QLineEdit::Normal, "", &ok);
        bool isNum = false;
        mobile.toULongLong(&isNum);
        if (!ok || mobile.length() != 10 || !isNum) {
            QMessageBox::warning(this, "Validation Error", "Mobile number must be exactly 10 digits.");
            return;
        }
        getBaseKiosk()->setPayment(new UPIAdapter());
    } else if (method == "Card") {
        QString cardLast4 = QInputDialog::getText(this, "Card Payment", "Enter card last 4 digits:", QLineEdit::Normal, "", &ok);
        bool isNum = false;
        cardLast4.toInt(&isNum);
        if (!ok || cardLast4.length() != 4 || !isNum) {
            QMessageBox::warning(this, "Validation Error", "Card pin must be exactly 4 digits.");
            return;
        }
        getBaseKiosk()->setPayment(new CardAdapter());
    }

    std::string txnId = "TXN" + QDateTime::currentDateTime().toString("mmsszzz").toStdString();

    for (int i = 0; i < qty; ++i) {
        Kiosk* base = getBaseKiosk();
        PurchaseItemCommand cmd(product.toStdString(),
                                kiosk, // Decorated kiosk for pre-checks
                                base->getInventory(),
                                base->getPayment(),
                                base->getDispenser(),
                                base->getPricingPolicy());
        cmd.execute();
        
        std::string logResult = cmd.getLog();
        logResult = "[User: " + currentUserId.toStdString() + "] [ID: " + txnId + "] " + logResult;
        TransactionLog txLog("data/transactions.csv");
        txLog.append(logResult);

        QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
        QString color = (logResult.find("SUCCESS") != std::string::npos) ? "#4caf50" : "#f44336";
        QString status = (logResult.find("SUCCESS") != std::string::npos) ? "SUCCESS" : "FAILED";

        QString logMsg = QString("<span style=\"color:#888888;\">[%1]</span> "
                                 "<span style=\"color:%2; font-weight:bold;\">%3</span>: %4 "
                                 "<br/><span style=\"color:#aaaaaa; font-size:10px;\">Transaction ID: %5</span>"
                                 "<br/><span style=\"color:#ffeb3b; font-size:10px;\">[Patterns: Command, Strategy, Adapter]</span>")
                             .arg(timeStr, color, status, QString::fromStdString(logResult), QString::fromStdString(txnId));

        custLogBox->append(logMsg);
    }

    
    if (method == "Wallet") {
        userStore->save();
        refreshWalletBalance();
    }
}

void MainWindow::onRefundClicked() {
    QString txn = transactionInput->text().trimmed();
    QString pid = refundProductIdInput->text().trimmed();
    if (txn.isEmpty() || pid.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter both Transaction ID and Product ID.");
        return;
    }

    // Check if user has made any purchases first
    TransactionLog txLogCheck("data/transactions.csv");
    auto lines = txLogCheck.readAll();
    bool userPurchased = false;
    for (const auto& l : lines) {
        if (l.find("[User: " + currentUserId.toStdString() + "]") != std::string::npos && 
            l.find("SUCCESS") != std::string::npos) {
            userPurchased = true;
            break;
        }
    }
    
    if (!userPurchased) {
        QMessageBox::warning(this, "Refund Error", "Nothing to refund, please buy something first.");
        return;
    }

    RefundCommand cmd(txn.toStdString(), pid.toStdString(), getBaseKiosk()->getPayment(), getBaseKiosk()->getInventory());
    cmd.execute();


    std::string logResult = cmd.getLog();
    TransactionLog txLog("data/transactions.csv");
    txLog.append(logResult);

    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString color = (logResult.find("REFUNDED") != std::string::npos) ? "#ff9800" : "#f44336";
    QString status = (logResult.find("REFUNDED") != std::string::npos) ? "REFUND" : "FAILED";

    QString logMsg = QString("<span style=\"color:#888888;\">[%1]</span> "
                             "<span style=\"color:%2; font-weight:bold;\">%3</span>: %4 "
                             "<br/><span style=\"color:#ffeb3b; font-size:10px;\">[Patterns: Command, Adapter]</span>")
                         .arg(timeStr, color, status, QString::fromStdString(logResult));

    custLogBox->append(logMsg);
    transactionInput->clear();
    refundProductIdInput->clear();
    
    userStore->save();
    refreshWalletBalance();
}

void MainWindow::onRestockClicked() {
    QString product = restockProductIdInput->text().trimmed();
    int qty = restockQtyInput->value();

    if (product.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter a product ID.");
        return;
    }

    RestockCommand cmd(product.toStdString(), qty, getBaseKiosk()->getInventory());
    cmd.execute();


    std::string logResult = cmd.getLog();
    TransactionLog txLog("data/transactions.csv");
    txLog.append(logResult);

    QString timeStr = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString color = (logResult.find("RESTOCK") != std::string::npos) ? "#2196f3" : "#f44336";
    QString status = (logResult.find("RESTOCK") != std::string::npos) ? "RESTOCK" : "FAILED";

    QString logMsg = QString("<span style=\"color:#888888;\">[%1]</span> "
                             "<span style=\"color:%2; font-weight:bold;\">%3</span>: %4 "
                             "<br/><span style=\"color:#ffeb3b; font-size:10px;\">[Patterns: Command, Proxy]</span>")
                         .arg(timeStr, color, status, QString::fromStdString(logResult));

    adminLogBox->append(logMsg);
    restockProductIdInput->clear();
    restockQtyInput->setValue(1);
}

void MainWindow::onViewStockClicked() {
    Kiosk* base = getBaseKiosk();
    if (!base || !base->getInventory()) return;
    adminLogBox->append(QString("<br/><span style=\"color:#2196f3; font-weight:bold;\">--- Current Stock Status ---</span>"));
    std::vector<std::string> products = base->getInventory()->getAllProductIds();
    for (const auto& p : products) {
        int stock = base->getInventory()->getStock(p);
        QString line = QString("Product ID: <b>%1</b> | Stock: <b>%2</b>")
                           .arg(QString::fromStdString(p)).arg(stock);
        adminLogBox->append(line);
    }
}


void MainWindow::onViewTransactionsClicked() {
    adminLogBox->append(QString("<br/><span style=\"color:#ff9800; font-weight:bold;\">--- Transaction Log (data/transactions.csv) ---</span>"));
    TransactionLog txLog("data/transactions.csv");
    auto lines = txLog.readAll();
    if (lines.empty()) {
        adminLogBox->append("<i>No transactions found or unable to read file.</i>");
    } else {
        for (const auto& l : lines) {
            adminLogBox->append(QString::fromStdString(l));
        }
    }
}

void MainWindow::refreshWalletBalance() {
    if (currentUserId.isEmpty()) return;
    double bal = UserWallet::getInstance()->getBalance(currentUserId.toStdString());
    walletBalanceLabel->setText(QString("Wallet Balance: Rs. %1").arg(bal, 0, 'f', 2));
}

void MainWindow::onTopUpWalletClicked() {
    bool ok;
    double amount = QInputDialog::getDouble(this, "Top Up Wallet", 
                                            "Enter amount to add:", 
                                            100.00, 1.00, 10000.00, 2, &ok);
    if (ok && amount > 0) {
        QStringList methods;
        methods << "UPI" << "Card";
        QString method = QInputDialog::getItem(this, "Top Up Method", "Select payment method:", methods, 0, false, &ok);
        if (!ok || method.isEmpty()) return;

        if (method == "UPI") {
            QString mobile = QInputDialog::getText(this, "UPI Payment", "Enter mobile number for UPI (10 digits):", QLineEdit::Normal, "", &ok);
            bool isNum = false;
            mobile.toULongLong(&isNum);
            if (!ok || mobile.length() != 10 || !isNum) {
                QMessageBox::warning(this, "Validation Error", "Mobile number must be exactly 10 digits.");
                return;
            }
        } else if (method == "Card") {
            QString cardLast4 = QInputDialog::getText(this, "Card Payment", "Enter card last 4 digits:", QLineEdit::Normal, "", &ok);
            bool isNum = false;
            cardLast4.toInt(&isNum);
            if (!ok || cardLast4.length() != 4 || !isNum) {
                QMessageBox::warning(this, "Validation Error", "Card pin must be exactly 4 digits.");
                return;
            }
        }

        UserWallet::getInstance()->topUp(currentUserId.toStdString(), amount, true);
        userStore->save();
        refreshWalletBalance();
        custLogBox->append(QString("<span style=\"color:#4caf50;\">[Wallet] Added Rs. %1 to wallet via %2.</span>").arg(amount).arg(method));
    }
}

void MainWindow::onRunDiagnosisClicked() {
    DiagnosisDialog dialog(kiosk, this);
    dialog.exec();
}

Kiosk* MainWindow::getBaseKiosk() {
    KioskInterface* current = kiosk;
    while (current) {
        Kiosk* base = dynamic_cast<Kiosk*>(current);
        if (base) return base;
        
        KioskModule* mod = dynamic_cast<KioskModule*>(current);
        if (mod) current = mod->getWrappedKiosk();
        else break;
    }
    return nullptr;
}