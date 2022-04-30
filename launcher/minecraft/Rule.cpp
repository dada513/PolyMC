/* Copyright 2013-2021 MultiMC Contributors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QJsonObject>
#include <QJsonArray>

#include "Rule.h"

RuleAction RuleAction_fromString(QString name)
{
    if (name == "allow")
        return Allow;
    if (name == "disallow")
        return Disallow;
    return Defer;
}

QList<std::shared_ptr<Rule>> rulesFromJsonV4(const QJsonObject &objectWithRules)
{
    QList<std::shared_ptr<Rule>> rules;
    auto rulesVal = objectWithRules.value("rules");
    if (!rulesVal.isArray())
        return rules;

    QJsonArray ruleList = rulesVal.toArray();
    for (auto ruleVal : ruleList)
    {
        std::shared_ptr<Rule> rule;
        if (!ruleVal.isObject())
            continue;
        auto ruleObj = ruleVal.toObject();
        auto actionVal = ruleObj.value("action");
        if (!actionVal.isString())
            continue;
        auto action = RuleAction_fromString(actionVal.toString());
        if (action == Defer)
            continue;

        auto osVal = ruleObj.value("os");
        if (!osVal.isObject())
        {
            // add a new implicit action rule
            rules.append(ImplicitRule::create(action));
            continue;
        }

        auto osObj = osVal.toObject();
        auto osNameVal = osObj.value("name");
        auto osArchVal = osObj.value("arch");
        if (!osNameVal.isString() && !osArchVal.isString())
            continue;
        // add a new OS rule
        rules.append(OsRule::create(action, osNameVal.toString(), osArchVal.toString()));
    }
    return rules;
}

QJsonObject ImplicitRule::toJson()
{
    QJsonObject ruleObj;
    ruleObj.insert("action", m_result == Allow ? QString("allow") : QString("disallow"));
    return ruleObj;
}

QJsonObject OsRule::toJson()
{
    QJsonObject ruleObj;
    ruleObj.insert("action", m_result == Allow ? QString("allow") : QString("disallow"));
    QJsonObject osObj;
    if(!m_system.isEmpty()) {
        osObj.insert("name", m_system);
    }
    if(!m_arch.isEmpty())
    {
        osObj.insert("arch", m_arch);
    }

    ruleObj.insert("os", osObj);
    return ruleObj;
}

