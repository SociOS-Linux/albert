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

#include <algorithm>
#include <vector>

namespace {

static inline double clamp01(double s)
{
    if (s < 0.0) return 0.0;
    if (s > 1.0) return 1.0;
    return s;
}

static inline double matchScore(const QString &query, const QString &candidate)
{
    albert::Matcher m(candidate);
    const albert::Match mm = m.match(query);
    if (!mm)
        return 0.0;
    return clamp01(mm.score());
}

static inline long long run(const QStringList &cmd)
{
    return albert::runDetachedProcess(cmd);
}

}  // namespace

class SourceOSPlugin final : public albert::ExtensionPlugin,
                             public albert::RankedQueryHandler
{
    ALBERT_PLUGIN

public:
    SourceOSPlugin() = default;

    QString synopsis(const QString &query) const override
    {
        (void)query;
        return "doctor | profile apply | status | sesh | tmux | k9s | lazygit | lazydocker | yazi";
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
        const QString q = context.query().trimmed();
        const bool empty = q.isEmpty();

        struct ActionDef {
            QString key;
            QString title;
            QString subtitle;
            QString grapheme;
            QStringList cmd;
            std::vector<QString> terms;
        };

        // NOTE: We intentionally call the installed `sourceos` helper CLI.
        // The workstation profile installs it into ~/.local/bin.
        const std::vector<ActionDef> actions = {
            {"status",
             "SourceOS: status",
             "Run workstation doctor (fast health check)",
             "✅",
             {"sourceos", "doctor"},
             {"status", "doctor", "check", "health"}},

            {"doctor",
             "SourceOS: doctor",
             "Run workstation checks",
             "🩺",
             {"sourceos", "doctor"},
             {"doctor", "check", "diagnose", "health"}},

            {"apply",
             "SourceOS: profile apply",
             "Apply workstation profile",
             "🧰",
             {"sourceos", "profile", "apply"},
             {"apply", "profile", "install", "bootstrap"}},

            {"sesh",
             "Open sesh",
             "tmux session manager",
             "🧵",
             {"sesh"},
             {"sesh", "session", "tmux"}},

            {"tmux",
             "Open tmux",
             "terminal multiplexer",
             "🧩",
             {"tmux"},
             {"tmux", "mux"}},

            {"k9s",
             "Open k9s",
             "Kubernetes TUI",
             "⎈",
             {"k9s"},
             {"k9s", "kube", "kubernetes"}},

            {"lazygit",
             "Open lazygit",
             "Git TUI",
             "🌿",
             {"lazygit"},
             {"lazygit", "git"}},

            {"lazydocker",
             "Open lazydocker",
             "Docker/Podman TUI",
             "🐳",
             {"lazydocker"},
             {"lazydocker", "docker", "podman"}},

            {"yazi",
             "Open yazi",
             "Terminal file manager",
             "📁",
             {"yazi"},
             {"yazi", "files", "file"}},
        };

        std::vector<albert::RankItem> out;
        out.reserve(actions.size());

        for (const auto &a : actions) {
            double score = 0.0;

            if (empty) {
                score = 0.01;
            } else {
                for (const auto &term : a.terms)
                    score = std::max(score, matchScore(q, term));

                if (score <= 0.0)
                    continue;
            }

            auto icon_factory = [g=a.grapheme]() {
                return albert::Icon::grapheme(g, 1.0);
            };

            std::vector<albert::Action> acts;
            acts.push_back(albert::Action{
                "run",
                "Run",
                [cmd=a.cmd]() { run(cmd); },
                true
            });

            auto item = albert::StandardItem::make(
                QString("sourceos.%1").arg(a.key),
                a.title,
                a.subtitle,
                icon_factory,
                std::move(acts),
                a.key
            );

            out.emplace_back(item, score);
        }

        return out;
    }
};
