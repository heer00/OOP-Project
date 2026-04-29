#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QSpinBox>
#include <QString>
#include <QLabel>
#include "core/Kiosk.h"
#include "persistence/UserStore.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onRoleCustomerClicked();
    void onRoleAdminClicked();
    void onBuyClicked();
    void onRefundClicked();
    void onRestockClicked();
    void onViewStockClicked();
    void onViewTransactionsClicked();
    void onTopUpWalletClicked();
    void onBackToMainClicked();
    void onRunDiagnosisClicked();
    void refreshWalletBalance();


private:
    Ui::MainWindow *ui;

    KioskInterface* kiosk;
    Kiosk* baseKiosk;

    UserStore* userStore;
    QString currentUserId;
    QString lastKioskType;
    
    // UI Elements managed programmatically
    QStackedWidget* mainStack;
    
    // Role Menu
    QWidget* roleMenuWidget;
    QComboBox* kioskTypeDropdown;
    QLineEdit* userIdInput;
    QPushButton* btnCustomer;
    QPushButton* btnAdmin;

    // Customer View
    QWidget* customerWidget;
    QLabel* walletBalanceLabel;
    QPushButton* btnTopUpWallet;
    QComboBox* productDropdown;
    QSpinBox* buyQtyInput;
    QPushButton* btnBuy;
    QLineEdit* transactionInput;
    QLineEdit* refundProductIdInput;
    QPushButton* btnRefund;
    QPushButton* btnCustBack;
    QTextEdit* custLogBox;

    // Admin View
    QWidget* adminWidget;
    QLineEdit* restockProductIdInput;
    QSpinBox* restockQtyInput;
    QPushButton* btnRestock;
    QPushButton* btnViewStock;
    QPushButton* btnViewTransactions;
    QPushButton* btnRunDiagnosis;
    QPushButton* btnAdminBack;

    QTextEdit* adminLogBox;
    
    // Simulation Controls
    QSpinBox* batterySlider;
    QPushButton* btnSetBattery;

    void setupProgrammaticUI();

    Kiosk* getBaseKiosk();

};

#endif