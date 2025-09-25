#include <QApplication>
#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextEdit>
#include <QListWidget>
#include <QFrame>
#include <QFont>
#include <QTimer>
#include <QPixmap>
#include <QLineEdit>

//Player Framework
struct Player 
{
    int id;
    QString codename;
    QString status;
};

//Text Helper
QString formatPlayer(const Player& p) 
{
    return QString("ID:%1  %2  - %3").arg(p.id).arg(p.codename).arg(p.status);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    //---------------- Splash Screen ----------------
    QWidget splash;
    splash.setWindowTitle("Laser Tag - Splash Screen");
    splash.setFixedSize(900, 600);

    QVBoxLayout splashLayout(&splash);
    QLabel splashImage;
    splashImage.setAlignment(Qt::AlignCenter);

    //Load Logo Image
    QPixmap pixmap("images/Logo.jpg");
    if (!pixmap.isNull()) 
    {
        splashImage.setPixmap(pixmap.scaled(splash.size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
    } else {
        splashImage.setText("Logo not found");
        splashImage.setStyleSheet("color: white; font-size: 20px;");
    }

    splashLayout.addWidget(&splashImage);



    //---------------- Main Screen ----------------
    QWidget mainScreen;
    mainScreen.setWindowTitle("Laser Tag - Main Screen");
    mainScreen.setFixedSize(900, 600);
    mainScreen.setStyleSheet("background-color: #1a1525; color: #f0f0f0;");

    QVBoxLayout mainLayout(&mainScreen);


    //---------------- Logo Image ----------------
    QLabel mainLogo;
    QPixmap mainPixmap("images/Logo_Simple.png");
    if (!mainPixmap.isNull()) 
    {
        mainLogo.setPixmap(mainPixmap.scaled(300, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else 
    {
        mainLogo.setText("Logo_Simple not found");
        mainLogo.setAlignment(Qt::AlignCenter);
        mainLogo.setStyleSheet("color: #f0f0f0; border: 1px solid #4a4f4d;");
        mainLogo.setFixedSize(300, 150);
    }
    mainLogo.setAlignment(Qt::AlignCenter);
    mainLayout.addWidget(&mainLogo, 0, Qt::AlignHCenter);

    
    //---------------- Top Divider ----------------
    QFrame logoDivider;
    logoDivider.setFrameShape(QFrame::HLine);
    logoDivider.setFrameShadow(QFrame::Plain);
    logoDivider.setStyleSheet(
        "color: #004a85; "
        "background-color: #004a85; "
        "margin-top: 32px; "
        "margin-bottom: 32px; "
        "height: 3px;"
    );
    mainLayout.addWidget(&logoDivider);


    //---------------- Upper Buttons ----------------
    //Start Game, End Game, Quit
    QHBoxLayout buttonLayout;
    QPushButton startGameBtn("Start Game");
    QPushButton endGameBtn("End Game");
    QPushButton quitBtn("Quit Application");
    buttonLayout.addWidget(&startGameBtn);
    buttonLayout.addWidget(&endGameBtn);
    buttonLayout.addWidget(&quitBtn);
    mainLayout.addLayout(&buttonLayout);


    //---------------- Lower Buttons ----------------
    //Input Box, Add Player, Remove Player, Clear Players
    QHBoxLayout addPlayerLayout;
    QLineEdit playerIdInput;
    playerIdInput.setPlaceholderText("Enter Player ID...");
    playerIdInput.setFixedWidth(200);
    addPlayerLayout.addWidget(&playerIdInput);

    QPushButton addPlayerBtn("Add Player");
    QPushButton removePlayerBtn("Remove Player");
    QPushButton clearPlayersBtn("Clear Players");
    addPlayerLayout.addWidget(&addPlayerBtn);
    addPlayerLayout.addWidget(&removePlayerBtn);
    addPlayerLayout.addWidget(&clearPlayersBtn);

    mainLayout.addLayout(&addPlayerLayout);


    //---------------- Bottom Divider ----------------
    QFrame divider;
    divider.setFrameShape(QFrame::HLine);
    divider.setFrameShadow(QFrame::Plain);
    divider.setStyleSheet(
        "color: #44125cff; "
        "background-color: #44125cff; "
        "margin-top: 64px; "
        "margin-bottom: 64px; "
        "height: 3px;"
    );
    mainLayout.addWidget(&divider);



    //---------------- Text Boxes ----------------
    QHBoxLayout boxesLayout;
    boxesLayout.setSpacing(12);

    //Status Box
    QFrame statusFrame;
    statusFrame.setFrameShape(QFrame::NoFrame);
    statusFrame.setStyleSheet("QFrame { border: 2px solid #403354; border-radius: 6px; background-color: #2a2139; }");
    statusFrame.setMinimumWidth(300);

    QVBoxLayout statusFrameLayout(&statusFrame);
    statusFrameLayout.setContentsMargins(8,8,8,8);
    statusFrameLayout.setSpacing(4);

    QTextEdit statusBox;
    statusBox.setReadOnly(true);
    statusBox.setFrameShape(QFrame::NoFrame);
    statusBox.setStyleSheet("background: transparent; color: #eaeaea; border: none;");
    statusBox.setText("Game status will appear here...");
    statusFrameLayout.addWidget(&statusBox);
    boxesLayout.addWidget(&statusFrame, 2);


    //Team A (Red) Box
    QFrame teamAFrame;
    teamAFrame.setFrameShape(QFrame::NoFrame);
    teamAFrame.setStyleSheet("QFrame { border: 3px solid #922f34; border-radius: 6px; background-color: #2a2139; }");
    teamAFrame.setMinimumWidth(220);

    QVBoxLayout teamALayout(&teamAFrame);
    teamALayout.setContentsMargins(6,6,6,6);
    teamALayout.setSpacing(4);

    QLabel teamALabel("Team A");
    QFont teamAFont = teamALabel.font();
    teamAFont.setBold(true);
    teamAFont.setUnderline(true);
    teamALabel.setFont(teamAFont);
    teamALabel.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");
    teamALabel.setAlignment(Qt::AlignCenter);

    QListWidget teamAList;
    teamAList.setFrameShape(QFrame::NoFrame);
    teamAList.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");

    teamALayout.addWidget(&teamALabel);
    teamALayout.addWidget(&teamAList);


    //Team B (Green) Box
    QFrame teamBFrame;
    teamBFrame.setFrameShape(QFrame::NoFrame);
    teamBFrame.setStyleSheet("QFrame { border: 3px solid #4fb269; border-radius: 6px; background-color: #2a2139; }");
    teamBFrame.setMinimumWidth(220);

    QVBoxLayout teamBLayout(&teamBFrame);
    teamBLayout.setContentsMargins(6,6,6,6);
    teamBLayout.setSpacing(4);

    QLabel teamBLabel("Team B");
    QFont teamBFont = teamBLabel.font();
    teamBFont.setBold(true);
    teamBFont.setUnderline(true);
    teamBLabel.setFont(teamBFont);
    teamBLabel.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");
    teamBLabel.setAlignment(Qt::AlignCenter);

    QListWidget teamBList;
    teamBList.setFrameShape(QFrame::NoFrame);
    teamBList.setStyleSheet("background: transparent; border: none; color: #f0f0f0;");

    teamBLayout.addWidget(&teamBLabel);
    teamBLayout.addWidget(&teamBList);

    boxesLayout.addWidget(&teamAFrame, 1);
    boxesLayout.addWidget(&teamBFrame, 1);
    mainLayout.addLayout(&boxesLayout);


    //---------------- Player Database (Fake Players) ----------------
    QList<Player> teamAPlayers;
    QList<Player> teamBPlayers;

    QList<Player> availablePlayers = 
    {
        {101, "Alice", "Ready"},
        {202, "Bob", "Ready"},
        {303, "Charlie", "Ready"},
        {404, "Xander", "Ready"},
        {505, "Yara", "Ready"},
        {606, "Zane", "Ready"}
    };


    //---------------- Button Actions ----------------
    //Quit
    QObject::connect(&quitBtn, &QPushButton::clicked, &app, &QApplication::quit);

    //Clear Players
    QObject::connect(&clearPlayersBtn, &QPushButton::clicked, [&]() 
    {
        teamAPlayers.clear();
        teamBPlayers.clear();
        teamAList.clear();
        teamBList.clear();
        statusBox.append("Players cleared from both teams.");
    });

    //Add Players
    QObject::connect(&addPlayerBtn, &QPushButton::clicked, [&]() 
    {
        bool ok;
        int enteredId = playerIdInput.text().toInt(&ok);
        if (!ok) 
        {
            statusBox.append("Invalid ID entered.");
            return;
        }

        auto it = std::find_if(availablePlayers.begin(), availablePlayers.end(), [&](const Player& p){ return p.id == enteredId; });

        if (it == availablePlayers.end()) 
        {
            statusBox.append("Player with ID " + QString::number(enteredId) + " not found.");
            return;
        }

        auto alreadyIn = [&](const QList<Player>& team)
        {
            return std::any_of(team.begin(), team.end(), [&](const Player& p){ return p.id == enteredId; });
        };

        if (alreadyIn(teamAPlayers) || alreadyIn(teamBPlayers)) 
        {
            statusBox.append("Player already in game.");
            return;
        }

        if (enteredId % 2 == 1) 
        {
            teamAPlayers.append(*it);
            teamAList.addItem(formatPlayer(*it));
            statusBox.append(it->codename + " added to Team A.");
        } else 
        {
            teamBPlayers.append(*it);
            teamBList.addItem(formatPlayer(*it));
            statusBox.append(it->codename + " added to Team B.");
        }
    });

    //Remove Players
    QObject::connect(&removePlayerBtn, &QPushButton::clicked, [&]() 
    {
        bool ok;
        int enteredId = playerIdInput.text().toInt(&ok);
        if (!ok) {
            statusBox.append("Invalid ID entered.");
            return;
        }

        auto removeFromTeam = [&](QList<Player>& team, QListWidget& list, const QString& teamName) 
        {
            for (int i = 0; i < team.size(); ++i) 
            {
                if (team[i].id == enteredId) {
                    statusBox.append(team[i].codename + " removed from " + teamName + ".");
                    team.removeAt(i);
                    list.clear();
                    for (const auto& p : team) list.addItem(formatPlayer(p));
                    return true;
                }
            }
            return false;
        };

        if (!removeFromTeam(teamAPlayers, teamAList, "Team A") &&
            !removeFromTeam(teamBPlayers, teamBList, "Team B")) 
            {
            statusBox.append("Player with ID " + QString::number(enteredId) + " not found in game.");
        }
    });

    //Start Game
    QObject::connect(&startGameBtn, &QPushButton::clicked, [&]() 
    {
        for (auto& p : teamAPlayers) p.status = "Active";
        for (auto& p : teamBPlayers) p.status = "Active";

        teamAList.clear();
        for (const auto& p : teamAPlayers) teamAList.addItem(formatPlayer(p));
        teamBList.clear();
        for (const auto& p : teamBPlayers) teamBList.addItem(formatPlayer(p));

        statusBox.append("Game started! All players set to Active.");
    });

    //End Game
    QObject::connect(&endGameBtn, &QPushButton::clicked, [&]() 
    {
        for (auto& p : teamAPlayers) p.status = "Ready";
        for (auto& p : teamBPlayers) p.status = "Ready";

        teamAList.clear();
        for (const auto& p : teamAPlayers) teamAList.addItem(formatPlayer(p));
        teamBList.clear();
        for (const auto& p : teamBPlayers) teamBList.addItem(formatPlayer(p));

        statusBox.append("Game ended. All players set to Ready.");
    });



    //---------------- Splash to Main Switch ----------------
    QTimer::singleShot(3000, [&]() 
    {
        splash.close();
        mainScreen.show();
        statusBox.append("Entered main screen.");
    });

    splash.show();
    return app.exec();
}