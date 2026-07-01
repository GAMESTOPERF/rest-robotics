# Contributing to REST Robotics Code

Welcome! This guide explains how to add or update code in this repo so everything stays organized and nothing breaks for other teams.

---

## Before You Start — The Golden Rules

1. **Never push broken code to `main`** — `main` is the "official" branch. Always test your code on the robot before pushing.
2. **Use branches** — Work on your own branch, then merge when it's ready (see below).
3. **Write a clear commit message** — "fixed stuff" is not helpful. "Fix servo not returning to center on reset" is.
4. **Don't commit secrets** — No WiFi passwords, API keys, or anything personal in the code.

---

## Step-by-Step: How to Add Code

### 1. Make sure you're up to date
```bash
git pull origin main
```
This downloads the latest changes from GitHub before you start — prevents conflicts.

### 2. Create a new branch for your work
```bash
git checkout -b your-name/what-youre-doing
```
Example: `git checkout -b franklin/fix-line-sensor-calibration`

A **branch** is like a personal copy of the repo where you can experiment safely. It doesn't affect `main` until you're ready.

### 3. Make your changes, save your `.ino` file

### 4. Stage and commit your changes
```bash
git add .
git commit -m "Short description of what you changed"
```

Good commit messages:
- `Add Bravo milestone code for auton team`
- `Fix motor direction reversal on turns`
- `Add PID tuning notes to tools/README`

Bad commit messages:
- `update`
- `asdfgh`
- `changes`

### 5. Push your branch to GitHub
```bash
git push origin your-name/what-youre-doing
```

### 6. Open a Pull Request on GitHub
Go to github.com → your repo → click **"Compare & pull request"** → describe what you changed → click **"Create pull request"**.

Someone else on the team should review it before it goes into `main`.

---

## Where Does My Code Go?

| What you're adding | Where it goes |
|---|---|
| Milestone Alpha code (RC team, 2025-2026) | `rc-team/2025-2026/milestones/alpha/` |
| Final competition code (V2X, 2025-2026) | `v2x-team/2025-2026/competition/` |
| A utility sketch all teams can use | `shared-libraries/` |
| Hardware wiring diagram or setup notes | `docs/` |

---

## Arduino File Naming

Each sketch folder name must match the `.ino` file name inside it. Arduino requires this.

```
milestones/
  alpha/
    alpha/           ← folder
      alpha.ino      ← .ino file with the SAME name as the folder
```

---

## Questions?

Ask in Slack or talk to the team lead before changing code in `shared-libraries/` — those changes affect everyone.
