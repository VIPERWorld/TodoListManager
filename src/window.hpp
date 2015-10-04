/**
 * \file
 *
 * \author Mattia Basaglia
 *
 * \copyright Copyright (C) 2015 Mattia Basaglia
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TLIM_WINDOW_HPP
#define TLIM_WINDOW_HPP

#include <QMainWindow>
#include <QEvent>
#include "ui_window.h"

namespace tlim {

/**
 * \brief Main window
 */
class Window : public QMainWindow, private Ui::Window
{
    Q_OBJECT

public:
    explicit Window(QWidget* parent = nullptr);
    ~Window();

    bool save_json(const QString& file_name) const;
    bool load_json(const QString& file_name) const;

protected:
    void changeEvent(QEvent* event) override;

private:
    void addBelow(QTreeWidgetItem* item);
    void addAbove(QTreeWidgetItem* item);
    void addChild(QTreeWidgetItem* item);
    void addTop(QTreeWidgetItem* item);
    void addBottom(QTreeWidgetItem* item);
};


} // namespace tlim

#endif // TLIM_WINDOW_HPP
