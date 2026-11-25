#pragma once
#include <QColor>
#include <QPalette>
#include <QString>

namespace Style {

inline QString applyTokens(QString text) {
    const char* const tokens[][2] = {
        { "@ACCENTHOVER", "#f1dcbd" },
        { "@ACCENTDARK", "#1b1509" },
        { "@ACCENT", "#e6c9a2" },
        { "@BORDERSOFT", "#222226" },
        { "@BORDER", "#2c2c32" },
        { "@SIDEBAR", "#111113" },
        { "@CARD2", "#222227" },
        { "@CARD", "#19191c" },
        { "@BG", "#0d0d0f" },
        { "@INPUT", "#121214" },
        { "@HOVER", "#27272d" },
        { "@SELECT", "#2a2520" },
        { "@TEXT", "#f1f1f1" },
        { "@MUTED", "#8e8e97" },
        { "@DISABLED", "#5a5a62" },
        { "@DANGERBG", "#2a1517" },
        { "@DANGER", "#ef6b70" },
        { "@SUCCESS", "#46c28e" }
    };
    const int count = static_cast<int>(sizeof(tokens) / sizeof(tokens[0]));
    for (int i = 0; i < count; ++i) {
        text.replace(QString::fromLatin1(tokens[i][0]), QString::fromLatin1(tokens[i][1]));
    }
    return text;
}

inline QString general() {
    return QString::fromUtf8(R"QSS(
QWidget { color: @TEXT; font-family: "Segoe UI", "Inter", "Noto Sans", sans-serif; font-size: 13px; }
QMainWindow, QDialog { background: @BG; }
QLabel { background: transparent; }
QToolTip { background: @CARD2; color: @TEXT; border: 1px solid @BORDER; padding: 6px 8px; }
QScrollArea { background: transparent; border: none; }
QMessageBox { background: @CARD; }
QMessageBox QLabel { color: @TEXT; font-size: 13px; }
QInputDialog QLabel { color: @TEXT; }

#sidebar { background: @SIDEBAR; border-right: 1px solid @BORDERSOFT; }
#topbar { background: transparent; border-bottom: 1px solid @BORDERSOFT; }
#brandTitle { font-size: 16px; font-weight: 700; color: @ACCENT; }
#brandSub { font-size: 11px; color: @MUTED; }
#brandLogo { background: @ACCENT; color: @ACCENTDARK; border-radius: 10px; font-size: 18px; font-weight: 700; }
#userCard { background: @CARD; border: 1px solid @BORDERSOFT; border-radius: 12px; }
#userName { font-size: 13px; font-weight: 700; }
#userRole { font-size: 11px; color: @MUTED; }
#avatar { background: @ACCENT; color: @ACCENTDARK; border-radius: 20px; font-size: 14px; font-weight: 700; }
#avatarBig { background: @ACCENT; color: @ACCENTDARK; border-radius: 30px; font-size: 22px; font-weight: 700; }
#pageTitle { font-size: 22px; font-weight: 700; }
#pageDate { color: @MUTED; font-size: 13px; }
#sectionTitle { font-size: 14px; font-weight: 700; }
#bigName { font-size: 20px; font-weight: 700; }
#muted { color: @MUTED; }
#mutedSmall { color: @MUTED; font-size: 11px; }
#statTitle { color: @MUTED; font-size: 11px; font-weight: 600; }
#statValue { font-size: 26px; font-weight: 700; }
#statValueAccent { font-size: 26px; font-weight: 700; color: @ACCENT; }
#infoKey { color: @MUTED; font-size: 12px; }
#infoValue { font-size: 13px; font-weight: 600; }
#emptyState { color: @MUTED; font-size: 14px; }
#warnBanner { background: #2a2113; border: 1px solid #4a3b1c; border-radius: 10px; color: #e0b44c; padding: 10px 14px; }
#dangerBanner { background: @DANGERBG; border: 1px solid #4a2428; border-radius: 10px; color: @DANGER; padding: 10px 14px; }
#noteLine { background: @INPUT; border: 1px solid @BORDERSOFT; border-radius: 8px; padding: 8px 10px; }
#scoreValue { font-size: 22px; font-weight: 700; color: @ACCENT; }
#rankBubble { background: @CARD2; border: 1px solid @BORDER; border-radius: 16px; font-weight: 700; color: @ACCENT; }

#card { background: @CARD; border: 1px solid @BORDERSOFT; border-radius: 14px; }
#optionCard { background: @CARD; border: 1px solid @BORDERSOFT; border-radius: 12px; }
#optionCard:hover { border: 1px solid @ACCENT; }
#heroPanel { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #2a2218, stop:1 #0f0f11); border-right: 1px solid @BORDERSOFT; }
#heroTitle { font-size: 30px; font-weight: 700; color: @ACCENT; }
#heroText { color: #b9b2a6; font-size: 14px; }
#heroLogo { background: @ACCENT; color: @ACCENTDARK; border-radius: 16px; font-size: 30px; font-weight: 700; }
#loginTitle { font-size: 22px; font-weight: 700; }
#fieldLabel { color: @MUTED; font-size: 11px; font-weight: 600; }
)QSS");
}

inline QString buttons() {
    return QString::fromUtf8(R"QSS(
QPushButton { background: @CARD2; border: 1px solid @BORDER; border-radius: 8px; padding: 8px 16px; font-weight: 600; }
QPushButton:hover { background: @HOVER; border: 1px solid #4a4338; }
QPushButton:pressed { background: @BORDER; }
QPushButton:disabled { color: @DISABLED; background: @CARD; border: 1px solid @BORDERSOFT; }
QPushButton[kind="primary"] { background: @ACCENT; color: @ACCENTDARK; border: 1px solid @ACCENT; }
QPushButton[kind="primary"]:hover { background: @ACCENTHOVER; border: 1px solid @ACCENTHOVER; }
QPushButton[kind="primary"]:pressed { background: #cdb08a; }
QPushButton[kind="primary"]:disabled { background: #3b362e; color: #77736b; border: 1px solid #3b362e; }
QPushButton[kind="danger"] { background: @DANGERBG; color: @DANGER; border: 1px solid #4a2428; }
QPushButton[kind="danger"]:hover { background: #3a1c1f; border: 1px solid #6a2f34; }
QPushButton[kind="danger"]:disabled { background: @CARD; color: @DISABLED; border: 1px solid @BORDERSOFT; }
QPushButton[kind="success"] { background: #14281f; color: @SUCCESS; border: 1px solid #1f4634; }
QPushButton[kind="success"]:hover { background: #1a3629; border: 1px solid #2a6247; }
QPushButton[kind="success"]:disabled { background: @CARD; color: @DISABLED; border: 1px solid @BORDERSOFT; }
QPushButton[kind="ghost"] { background: transparent; border: 1px solid @BORDER; }
QPushButton[kind="ghost"]:hover { background: @HOVER; }
QPushButton[kind="ghost"]:disabled { background: transparent; }
QPushButton[kind="big"] { padding: 12px 20px; font-size: 14px; }

QPushButton#navButton { text-align: left; padding: 11px 14px; background: transparent; border: none; border-left: 3px solid transparent; border-radius: 8px; color: @MUTED; font-weight: 600; }
QPushButton#navButton:hover { background: @HOVER; color: @TEXT; }
QPushButton#navButton:checked { background: @SELECT; color: @ACCENT; border-left: 3px solid @ACCENT; }

QPushButton#chip { padding: 8px 0px; background: @INPUT; border: 1px solid @BORDER; border-radius: 8px; font-weight: 600; min-width: 62px; }
QPushButton#chip:hover { border: 1px solid @ACCENT; }
QPushButton#chip:checked { background: @ACCENT; color: @ACCENTDARK; border: 1px solid @ACCENT; }

QPushButton#arrowButton { padding: 6px 0px; min-width: 34px; max-width: 34px; font-size: 16px; }
)QSS");
}

inline QString inputs() {
    return QString::fromUtf8(R"QSS(
QLineEdit, QComboBox, QDateEdit, QTimeEdit, QDateTimeEdit, QSpinBox, QDoubleSpinBox, QTextEdit, QPlainTextEdit { background: @INPUT; border: 1px solid @BORDER; border-radius: 8px; padding: 8px 10px; selection-background-color: @ACCENT; selection-color: @ACCENTDARK; min-height: 18px; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus, QTimeEdit:focus, QDateTimeEdit:focus, QSpinBox:focus, QDoubleSpinBox:focus, QTextEdit:focus { border: 1px solid @ACCENT; }
QLineEdit:disabled, QComboBox:disabled, QDateEdit:disabled, QTimeEdit:disabled, QSpinBox:disabled, QDoubleSpinBox:disabled { color: @DISABLED; background: @CARD; }

QComboBox::drop-down, QDateEdit::drop-down, QDateTimeEdit::drop-down { subcontrol-origin: padding; subcontrol-position: center right; width: 28px; border: none; }

QComboBox::down-arrow {
    image: url("data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIxNiIgaGVpZ2h0PSIxNiIgdmlld0JveD0iMCAwIDE2IDE2IiBmaWxsPSJub25lIiBzdHJva2U9IiM4ZThlOTciIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIj4KICA8cG9seWxpbmUgcG9pbnRzPSI0IDYgOCAxMCAxMiA2Ii8+Cjwvc3ZnPg==");
    width: 14px; height: 14px; margin-right: 8px;
}
QDateEdit::down-arrow, QDateTimeEdit::down-arrow {
    image: url("data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHdpZHRoPSIxNiIgaGVpZ2h0PSIxNiIgdmlld0JveD0iMCAwIDI0IDI0IiBmaWxsPSJub25lIiBzdHJva2U9IiM4ZThlOTciIHN0cm9rZS13aWR0aD0iMiIgc3Ryb2tlLWxpbmVjYXA9InJvdW5kIiBzdHJva2UtbGluZWpvaW49InJvdW5kIj4KICA8cmVjdCB4PSIzIiB5PSI0IiB3aWR0aD0iMTgiIGhlaWdodD0iMTgiIHJ4PSIyIiByeT0iMiIvPgogIDxsaW5lIHgxPSIxNiIgeTE9IjIiIHgyPSIxNiIgeTI9IjYiLz4KICA8bGluZSB4MT0iOCIgeTE9IjIiIHgyPSI4IiB5Mj0iNiIvPgogIDxsaW5lIHgxPSIzIiB5MT0iMTAiIHgyPSIyMSIgeTI9IjEwIi8+Cjwvc3ZnPg==");
    width: 14px; height: 14px; margin-right: 8px;
}

QComboBox QAbstractItemView { background: @CARD2; border: 1px solid @BORDER; selection-background-color: @SELECT; selection-color: @ACCENT; outline: 0; padding: 4px; }
QAbstractSpinBox::up-button, QAbstractSpinBox::down-button { width: 18px; border: none; background: transparent; }

QCalendarWidget QWidget { background: @CARD; color: @TEXT; }
QCalendarWidget QAbstractItemView { background: @CARD; selection-background-color: @ACCENT; selection-color: @ACCENTDARK; outline: 0; }
QCalendarWidget QToolButton { background: transparent; color: @TEXT; border: none; padding: 6px 10px; font-weight: 600; }
QCalendarWidget QToolButton:hover { background: @HOVER; }
QCalendarWidget QMenu { background: @CARD2; }
QCalendarWidget QSpinBox { background: @INPUT; }

QCheckBox { spacing: 8px; background: transparent; }
QRadioButton { spacing: 8px; background: transparent; }

QMenu { background: @CARD2; border: 1px solid @BORDER; padding: 6px; }
QMenu::item { padding: 8px 18px; border-radius: 6px; }
QMenu::item:selected { background: @SELECT; color: @ACCENT; }
)QSS");
}

inline QString tablesAndLists() {
    return QString::fromUtf8(R"QSS(
QTableWidget, QTableView { background: @CARD; alternate-background-color: #1d1d21; border: 1px solid @BORDERSOFT; border-radius: 12px; gridline-color: transparent; selection-background-color: @SELECT; selection-color: @TEXT; outline: 0; }
QTableWidget::item { padding: 4px 10px; border-bottom: 1px solid @BORDERSOFT; }
QTableWidget::item:selected { background: @SELECT; color: @ACCENT; }
QHeaderView { background: transparent; }
QHeaderView::section { background: @CARD; color: @MUTED; border: none; border-bottom: 1px solid @BORDER; padding: 10px 10px; font-size: 11px; font-weight: 700; }
QTableCornerButton::section { background: @CARD; border: none; }

QListWidget { background: @CARD; border: 1px solid @BORDERSOFT; border-radius: 12px; padding: 6px; outline: 0; }
QListWidget::item { padding: 10px 12px; border-radius: 8px; margin-bottom: 2px; }
QListWidget::item:hover { background: @HOVER; }
QListWidget::item:selected { background: @SELECT; color: @ACCENT; }

QScrollBar:vertical { background: transparent; width: 12px; margin: 2px; }
QScrollBar::handle:vertical { background: #3a3a42; border-radius: 4px; min-height: 32px; }
QScrollBar::handle:vertical:hover { background: #50505a; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }
QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }
QScrollBar:horizontal { background: transparent; height: 12px; margin: 2px; }
QScrollBar::handle:horizontal { background: #3a3a42; border-radius: 4px; min-width: 32px; }
QScrollBar::handle:horizontal:hover { background: #50505a; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }
QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }

QTabWidget::pane { border: none; top: 0px; }
QTabBar { background: transparent; }
QTabBar::tab { background: transparent; color: @MUTED; padding: 11px 20px; border: none; border-bottom: 2px solid transparent; font-weight: 600; margin-right: 4px; }
QTabBar::tab:hover { color: @TEXT; }
QTabBar::tab:selected { color: @ACCENT; border-bottom: 2px solid @ACCENT; }

QProgressBar { background: @INPUT; border: none; border-radius: 4px; max-height: 8px; min-height: 8px; text-align: center; color: transparent; }
QProgressBar::chunk { background: @ACCENT; border-radius: 4px; }
)QSS");
}

inline QString badges() {
    return QString::fromUtf8(R"QSS(
QLabel#badge { border-radius: 10px; padding: 3px 11px; font-size: 11px; font-weight: 700; background: #2a2a2e; color: #b5b5bb; }
QLabel#badge[kind="pending"] { background: #2e2716; color: #e0b44c; }
QLabel#badge[kind="confirmed"] { background: #142a21; color: #46c28e; }
QLabel#badge[kind="arrived"] { background: #13293a; color: #55b2e6; }
QLabel#badge[kind="noshow"] { background: #3a1a1d; color: #ef6b70; }
QLabel#badge[kind="cancelled"] { background: #26262a; color: #85858d; }
QLabel#badge[kind="paid"] { background: #33281a; color: #e6c9a2; }
QLabel#badge[kind="gold"] { background: #3a2f12; color: #f0c850; }
QLabel#badge[kind="silver"] { background: #2b2d33; color: #c5c9d4; }
QLabel#badge[kind="bronze"] { background: #34241a; color: #d99a6c; }
QLabel#badge[kind="regular"] { background: #1f2630; color: #8fb0d9; }
QLabel#badge[kind="new"] { background: #142a21; color: #46c28e; }
QLabel#badge[kind="banned"] { background: #3a1a1d; color: #ef6b70; }
QLabel#badge[kind="top"] { background: #3a2f12; color: #f0c850; }
QLabel#badge[kind="standard"] { background: #1f2630; color: #8fb0d9; }
QLabel#badge[kind="junior"] { background: #2a2a2e; color: #b5b5bb; }
QLabel#badge[kind="role"] { background: #33281a; color: #e6c9a2; }
)QSS");
}

inline QString sheet() {
    return applyTokens(general() + buttons() + inputs() + tablesAndLists() + badges());
}

inline QPalette darkPalette() {
    QPalette palette;
    palette.setColor(QPalette::Window, QColor("#0d0d0f"));
    palette.setColor(QPalette::WindowText, QColor("#f1f1f1"));
    palette.setColor(QPalette::Base, QColor("#121214"));
    palette.setColor(QPalette::AlternateBase, QColor("#1d1d21"));
    palette.setColor(QPalette::Text, QColor("#f1f1f1"));
    palette.setColor(QPalette::Button, QColor("#222227"));
    palette.setColor(QPalette::ButtonText, QColor("#f1f1f1"));
    palette.setColor(QPalette::ToolTipBase, QColor("#222227"));
    palette.setColor(QPalette::ToolTipText, QColor("#f1f1f1"));
    palette.setColor(QPalette::Highlight, QColor("#e6c9a2"));
    palette.setColor(QPalette::HighlightedText, QColor("#1b1509"));
    palette.setColor(QPalette::PlaceholderText, QColor("#5f5f68"));
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor("#5a5a62"));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor("#5a5a62"));
    return palette;
}

}
