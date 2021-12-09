/*
 * Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
 *
 * Author:     songwentao <songwentao@uniontech.com>
 *
 * Maintainer: songwentao <songwentao@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This program aims to cache the the icon and name of apps to the hash table,
 * which can decrease the repeated resource consumption of loading the app info in the
 * running time.
 */

#include "menudialog.h"

#include <QApplication>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>

Menu::Menu(QWidget *parent)
    : QMenu(parent)
{
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint | Qt::Dialog);
    setAccessibleName("popmenu");
    setObjectName("rightMenu");
    qApp->installEventFilter(this);
}

/** 右键菜单显示后在很多场景下都需要隐藏，为避免在各个控件中分别做隐藏右键菜单窗口的处理，
 *  因此这里统一做了处理。
 * @brief Menu::eventFilter
 * @param watched 过滤器监听对象
 * @param event 过滤器事件对象
 * @return 返回true, 事件不再向下传递，返回false，事件向下传递
 */
bool Menu::eventFilter(QObject *watched, QEvent *event)
{
    // 存在rightMenu和rightMenuWindow的对象名
    if (!watched->objectName().startsWith("rightMenu") && isVisible()) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                hide();
                return true;
            }
        } else if (event->type() == QEvent::DragMove || event->type() == QEvent::Wheel
                   || event->type() == QEvent::Move) {
            // 按下应用拖动，按下应用从菜单上方移动时，鼠标滚轮滚动时，隐藏右键菜单
            hide();
        }

        // 当右键菜单显示时捕获鼠标的release事件,click=press+release，
        // 让click无效，从而让启动器窗口不关闭
        if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->source() == Qt::MouseEventSynthesizedByQt)
                return true;
        }

        /*
         * 1.处理当右键菜单显示时,esc按下，关闭右键菜单
         * 2.右键菜单出现后支持键盘上下选中菜单项,支持键盘点击回车键触发右键菜单选项
        */
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

            int size = actions().size();
            if (size <= 0)
                return false;

            switch (keyEvent->key()) {
            case Qt::Key_Escape:
                hide();
                return true;
            case Qt::Key_Up:
                moveUp(size);
                return true;
            case Qt::Key_Down:
                moveDown();
                return true;
            case Qt::Key_Return:
            case Qt::Key_Enter:
                // Qt::Key_Return[center enter], Qt::Key_Enter[right enter]
                openItem();
                return true;
            default:
                break;
            }
        }
    }

    return QMenu::eventFilter(watched, event);
}

void Menu::moveDown(int size)
{
    Q_UNUSED(size);
    QAction *activeAction = this->activeAction();
    const int index = this->actions().indexOf(activeAction);

    for(int i = index + 1; i > -1; ++i) {
        // 循环一遍后找不到可用的action，直接退出
        if (i == index) {
            return;
        }

        // 从头查找
        if (i >= this->actions().size()) {
            i = 0;
        }

        auto act = this->actions().at(i);
        if (act->isSeparator() || !act->isEnabled()) {
            continue;
        }

        setActiveAction(act);
        break;
    }
}

void Menu::moveUp(int size)
{
    Q_UNUSED(size);
    QAction *activeAction = this->activeAction();
    const int index = this->actions().indexOf(activeAction);

    for(int i = index - 1; i < this->actions().size(); --i) {
        // 循环一遍后找不到可用的action，直接退出
        if (i == index) {
            return;
        }

        // 从末尾查找
        if (i <= -1) {
            i = this->actions().size() - 1;
        }

        auto act = this->actions().at(i);
        if (act->isSeparator() || !act->isEnabled()) {
            continue;
        }

        setActiveAction(act);
        break;
    }
}

void Menu::openItem()
{
    QAction *activeAction = this->activeAction();
    int index = this->actions().indexOf(activeAction);
    if (index != -1)
        activeAction->triggered();
}