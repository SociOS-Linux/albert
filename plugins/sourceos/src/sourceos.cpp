// SPDX-FileCopyrightText: 2026 SociOS-Linux
// SPDX-License-Identifier: MIT

#include <albert/extensionplugin.h>
#include <albert/rankedqueryhandler.h>
#include <albert/matcher.h>
#include <albert/standarditem.h>
#include <albert/icon.h>
#include <albert/systemutil.h>

#include <QString>
#include <QStringList>
#include <vector>

namespace {

static inline double scoreMatch(const albert::Match &m)
{
    if (!m)
        return 0.0;
    // m.score() is [0,1] for matches; clamp defensively.
    const double s = m.score();
    return s < 0.0 ? 0.0 : (s > 1.0 ? 1.0 : s);
}

static inline std::vector<QString> keywords(const QString &s)
{
    // Keep it simple: keyword list used by matcher scoring.
    // (We can evolve to richer tokenization later.)
    return {s};
}

}  // namespace

class SourceOSPlugin final : public albert::ExtensionPlugin,
                             public albert::RankedQueryHandler
{
    ALBERT_PLUGIN

public:
    SourceOSPlugin() = default;

    QString id() const override { return "sourceos"; }
    QString name() const override { return "SourceOS"; }
    QString description() const override { return "SourceOS workstation actions (doctor, profiles, tools)"; }

    QString synopsis(const QString &query) const override
    {
        Q_UNUSED(query);
        return "doctor | profile apply | k9s | lazygit";
    }

    QString defaultTrigger() const override
    {
        return "sourceos ";
    }

    bool supportsFuzzyMatching() const override
    {
        return true;
    }

    std::vector<albert::RankItem> rankItems(albert::QueryContext &context) override
    {
        // This runs in a worker thread (RankedQueryHandler contract).
        const QString q = context.query().trimmed();

        struct ActionDef {
            QString key;        // query keyword
            QString title;      // item label
            QString subtitle;   // item subtext
            QString grapheme;   // icon
            QStringList cmd;    // commandline
        };

        const std::vector<ActionDef> actions = {
            {"doctor", "SourceOS: doctor", "Run workstation checks (workstation-v0)", "🩺", {"sourceos", "doctor", "workstation-v0"}},
            {"apply",  "SourceOS: profile apply", "Apply workstation profile (workstation-v0)", "🧰", {"sourceos", "profile", "apply", "workstation-v0"}},
            {"k9s",    "Open k9s", "Kubernetes TUI", "⎈", {"k9s"}},
            {"lazygit", "Open lazygit", "Git TUI", "🌿", {"lazygit"}},
        };

        // Empty query: show all actions with score 0.
        const bool empty = q.isEmpty();

        std::vector<albert::RankItem> out;
        out.reserve(actions.size());

        for (const auto &a : actions) {
            double score = 0.0;
            if (!empty) {
                albert::Matcher m(a.key);
                score = scoreMatch(m.match(q));
                if (score <= 0.0)
                    continue;
            }

            auto icon = [g=a.grapheme]() {
                return albert::Icon::grapheme(g, 1.0);
            };

            std::vector<albert::Action> acts;
            acts.push_back(albert::Action{
                "run",
                "Run",
                [cmd=a.cmd]() {
                    albert::runDetachedProcess(cmd);
                },
                true
            });

            auto item = albert::StandardItem::make(
                QString("sourceos.%1").arg(a.key),
                a.title,
                a.subtitle,
                icon,
                std::move(acts),
                a.key
            );

            // RankedQueryHandler expects score in (0,1]. For empty query we use 0, but the
            // docs say empty string yields score 0; core handles this.
            out.emplace_back(item, empty ? 0.0 : score);
        }

        return out;
    }
};

ALBERT_PLUGIN

// Qt plugin entry point
#include "sourceos.moc"
