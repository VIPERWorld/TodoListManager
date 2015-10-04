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
 *
 */

#include "window.hpp"
#include <QStandardPaths>
#include <QDir>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using  boost::property_tree::ptree;

namespace tlim {

enum Position {
    Below,
    Above,
    Child,
    Top,
    Bottom
};

void tree_widget_item_to_ptree(const QTreeWidgetItem* item, ptree& output)
{
    for ( int i = 0; i < item->columnCount(); i++ )
        output.add("data", item->data(i, Qt::EditRole).toString().toStdString());

    if ( item->checkState(0) == Qt::Checked )
        output.put("checked", true);

    ptree children;
    for ( int i = 0; i < item->childCount(); i++ )
    {
        ptree child;
        tree_widget_item_to_ptree(item->child(i), child);
        children.push_back({"", child});
    }
    if ( !children.empty() )
        output.add_child("children", children);
}

void tree_widget_item_from_ptree(QTreeWidgetItem* item, const ptree& input)
{
    item->setData(0, Qt::EditRole, QString::fromStdString(input.get("data", "")));
    item->setCheckState(0, input.get("checked", false) ? Qt::Checked : Qt::Unchecked);

    for ( const auto& child : input.get_child("children", {}) )
    {
        tree_widget_item_from_ptree(new QTreeWidgetItem(item), child.second);
    }
}

Window::Window(QWidget* parent)
    : QMainWindow(parent)
{
    setupUi(this);
    action_quit->setShortcut(QKeySequence::Quit);

    connect(button_add, &QAbstractButton::clicked, [this]{
        QString text = text_new->text();
        if ( text.isEmpty() )
            text = text_new->placeholderText();
        text_new->setFocus();
        text_new->selectAll();

        QTreeWidgetItem *item = new QTreeWidgetItem({text});
        item->setFlags(item->flags()|Qt::ItemIsUserCheckable|Qt::ItemIsEditable);
        item->setCheckState(0, Qt::Unchecked);
        switch ( Position(combo_position->currentIndex()) )
        {
            case Below: addBelow(item); break;
            case Above: addAbove(item); break;
            case Child: addChild(item); break;
            case Top:   addTop(item);   break;
            case Bottom:addBottom(item);break;
        }
        tree_view->setCurrentItem(item);
    });

    connect(tree_view->model(), &QAbstractItemModel::rowsInserted,
    [this](const QModelIndex &parent, int first, int last) {
        auto parent_item = tree_view->itemFromIndex(parent);
        if ( !parent_item )
            parent_item = tree_view->invisibleRootItem();
        tree_view->setCurrentItem(parent_item->child(last));
    });

    load_json(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/autosave.json");
}

Window::~Window()
{
    auto path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if ( !QDir().exists(path) )
        QDir().mkpath(path);

    save_json(path+"/autosave.json");
}

void Window::changeEvent(QEvent* event)
{
    if ( event->type() == QEvent::LanguageChange )
    {
        retranslateUi(this);
    }

    QMainWindow::changeEvent(event);
}

bool Window::save_json(const QString& file_name) const
{
    std::ofstream stream(file_name.toStdString());
    if ( !stream )
        return false;
    try {
        ptree tree;
        tree_widget_item_to_ptree(tree_view->invisibleRootItem(), tree);
        boost::property_tree::json_parser::write_json(stream, tree);
        return true;
    } catch ( const std::exception& ) {
        return false;
    }
}

bool Window::load_json(const QString& file_name) const
{
    std::ifstream stream(file_name.toStdString());
    if ( !stream )
        return false;
    try {
        ptree tree;
        boost::property_tree::json_parser::read_json(stream, tree);
        tree_widget_item_from_ptree(tree_view->invisibleRootItem(), tree);
        tree_view->expandToDepth(1);
        tree_view->setCurrentItem(tree_view->topLevelItem(tree_view->topLevelItemCount()-1));
        return true;
    } catch ( const std::exception& ) {
        return false;
    }

}

void Window::addAbove(QTreeWidgetItem* item)
{
    auto sibling = tree_view->currentItem();
    if ( !sibling )
        return addTop(item);
    auto parent = sibling->parent();
    if ( !parent )
        parent = tree_view->invisibleRootItem();
    parent->insertChild(parent->indexOfChild(sibling), item);
}

void Window::addBelow(QTreeWidgetItem* item)
{
    auto sibling = tree_view->currentItem();
    if ( !sibling )
        return addBottom(item);
    auto parent = sibling->parent();
    if ( !parent )
        parent = tree_view->invisibleRootItem();
    parent->insertChild(parent->indexOfChild(sibling)+1, item);
}

void Window::addChild(QTreeWidgetItem* item)
{
    auto parent = tree_view->currentItem();
    if ( !parent )
        return addBottom(item);
    parent->insertChild(tree_view->currentItem()->childCount(), item);
}

void Window::addBottom(QTreeWidgetItem* item)
{
    tree_view->insertTopLevelItem(tree_view->topLevelItemCount(), item);
}

void Window::addTop(QTreeWidgetItem* item)
{
    tree_view->insertTopLevelItem(0, item);
}

} // namespace tlim
