# JSON Schemas

## Encounter (`schema = 2`)

```json
{
  "schema": 2,
  "round": 1,
  "turnIndex": 0,
  "combatants": [
    {
      "id": 1,
      "name": "Vaelen",
      "initiative": 17,
      "dexMod": 3,
      "isPC": true,
      "conscious": true,
      "hp": 27,
      "ac": 17,
      "deathSaves": {"successes": 0, "failures": 0, "dead": false, "stable": false},
      "conditions": [{"name": "Bless", "remainingRounds": 8}],
      "notes": "Focus fire on the wight."
    }
  ]
}
```

## Characters (`schema = 2`)

```json
{
  "schema": 2,
  "characters": [
    {
      "name": "Bandit",
      "dexMod": 2,
      "isPC": false,
      "tags": ["human", "bandit"],
      "defaultHP": 11,
      "defaultAC": 12,
      "defaultNotes": ""
    }
  ]
}
```

## Groups (`schema = 2`)

```json
{
  "schema": 2,
  "groups": [
    {
      "name": "Bandits x6",
      "entries": [
        {"character": "Bandit", "count": 6}
      ]
    }
  ],
  "naming": {
    "pattern": "%name %index",
    "startIndex": 1,
    "zeroPad": true,
    "width": 2
  }
}
```

