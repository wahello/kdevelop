/*
    SPDX-FileCopyrightText: 2019 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "quickopenembeddedwidgetcombiner.h"

#include <QVBoxLayout>

using namespace KDevelop;

QuickOpenEmbeddedWidgetInterface* toInterface(QObject *object)
{
    return qobject_cast<QuickOpenEmbeddedWidgetInterface*>(object);
}

class KDevelop::QuickOpenEmbeddedWidgetCombinerPrivate
{
public:
    QuickOpenEmbeddedWidgetInterface* currentChild = nullptr;

    bool init(const QObjectList& children)
    {
        if (currentChild)
            return true;

        currentChild = nextChild(Next, children);
        return currentChild;
    }

    enum Action
    {
        Next,
        Previous,
        Up,
        Down,
    };
    QuickOpenEmbeddedWidgetInterface* nextChild(Action action, const QObjectList& children) const
    {
        if (action == Next || action == Down) {
            return nextChild(children.begin(), children.end());
        } else {
            return nextChild(children.rbegin(), children.rend());
        }
    }

    template<typename It>
    QuickOpenEmbeddedWidgetInterface* nextChild(const It start, const It end) const
    {
        if (start == end)
            return nullptr;

        auto current = start;
        if (currentChild) {
            current = std::find_if(start, end, [this](QObject *child) {
                return toInterface(child) == currentChild;
            });
        }

        auto it = std::find_if(current + 1, end, toInterface);
        if (it == end && current != start) {
            // cycle
            it = std::find_if(start, current, toInterface);
        }
        return (it != end) ? toInterface(*it) : nullptr;
    }

    bool navigate(Action action, const QObjectList& children)
    {
        if (!init(children)) {
            return false;
        }

        auto applyAction = [action](QuickOpenEmbeddedWidgetInterface* interface)
        {
            switch (action) {
            case Next:
                return interface->next();
            case Previous:
                return interface->previous();
            case Up:
                return interface->up();
            case Down:
                return interface->down();
            }
            Q_UNREACHABLE();
        };

        if (applyAction(currentChild)) {
            return true;
        }

        if (auto *next = nextChild(action, children)) {
            currentChild->resetNavigationState();
            currentChild = next;
            auto ret = applyAction(currentChild);
            return ret;
        }
        return false;
    }
};

QuickOpenEmbeddedWidgetCombiner::QuickOpenEmbeddedWidgetCombiner(QWidget* parent)
    : QWidget(parent)
    , d_ptr(new QuickOpenEmbeddedWidgetCombinerPrivate)
{
    setLayout(new QVBoxLayout);
    layout()->setContentsMargins(2, 2, 2, 2);
    layout()->setSpacing(0);
}

QuickOpenEmbeddedWidgetCombiner::~QuickOpenEmbeddedWidgetCombiner() = default;

bool QuickOpenEmbeddedWidgetCombiner::next()
{
    Q_D(QuickOpenEmbeddedWidgetCombiner);

    return d->navigate(QuickOpenEmbeddedWidgetCombinerPrivate::Next, children());
}

bool QuickOpenEmbeddedWidgetCombiner::previous()
{
    Q_D(QuickOpenEmbeddedWidgetCombiner);

    return d->navigate(QuickOpenEmbeddedWidgetCombinerPrivate::Previous, children());
}

bool QuickOpenEmbeddedWidgetCombiner::up()
{
    Q_D(QuickOpenEmbeddedWidgetCombiner);

    return d->navigate(QuickOpenEmbeddedWidgetCombinerPrivate::Up, children());
}

bool QuickOpenEmbeddedWidgetCombiner::down()
{
    Q_D(QuickOpenEmbeddedWidgetCombiner);

    return d->navigate(QuickOpenEmbeddedWidgetCombinerPrivate::Down, children());
}

void QuickOpenEmbeddedWidgetCombiner::back()
{
    Q_D(QuickOpenEmbeddedWidgetCombiner);

    if (d->currentChild) {
        d->currentChild->back();
    }
}

void QuickOpenEmbeddedWidgetCombiner::accept()
{
    Q_D(QuickOpenEmbeddedWidgetCombiner);

    if (d->currentChild) {
        d->currentChild->accept();
    }
}

void QuickOpenEmbeddedWidgetCombiner::resetNavigationState()
{
    for (auto* child : children()) {
        if (auto* interface = toInterface(child)) {
            interface->resetNavigationState();
        }
    }
}
