# GitHub Setup Guide for REST Robotics
### Step-by-step for Windows — written for beginners

---

## Part 1: What Is GitHub, Actually?

Before diving in, here's what the terms mean in plain English:

**Repository (repo)**
A folder that GitHub tracks. Every change you make to files inside it gets recorded with a timestamp and author. Think of it like Google Docs "version history" but for an entire folder.

**Commit**
A saved snapshot of your changes. When you commit, you write a short message describing what you did. Git stores that forever. If you break something, you can go back to any previous commit.

**Branch**
A parallel copy of the repo where you can work without affecting the main code. When your work is done and tested, you "merge" it back into `main`.

**Push**
Sending your local commits (on your computer) up to GitHub.com so others can see and download them.

**Pull**
Downloading commits from GitHub.com to your local computer.

**Clone**
Copying a GitHub repo down to your computer for the first time.

---

## Part 2: Create a GitHub Account

1. Go to **https://github.com**
2. Click **Sign up**
3. Use your school email (or personal — either works)
4. Choose a username the team will recognize (e.g., `franklin-rhs` or `rest-robotics-rhs`)
5. Verify your email

**Tip:** GitHub is free for public repos and also free for students — sign up for the [GitHub Student Developer Pack](https://education.github.com/pack) to get free private repos and other tools.

---

## Part 3: Create the Repository on GitHub

1. Log in to **github.com**
2. Click the **+** icon (top right) → **New repository**
3. Fill in:
   - **Repository name:** `rest-robotics`
   - **Description:** `Arduino code for REST Robotics — RC, V2X, and Autonomous teams`
   - **Visibility:** Public (or Private if you prefer — private still works fine)
   - **Do NOT check** "Add a README file" — you already have one
4. Click **Create repository**
5. GitHub shows you a page with setup commands — leave this open, you'll use it in a moment

---

## Part 4: Set Up Git on Your Computer

You said you have Git installed already. Let's confirm and configure it.

Open **Git Bash** (search for it in the Start menu):

```bash
# Check Git is installed
git --version
# Should print something like: git version 2.43.0.windows.1

# Set your name and email (uses these in commit history)
git config --global user.name "Franklin Zini"
git config --global user.email "your-email@example.com"
```

---

## Part 5: Get the Template Files onto Your Computer

The `GitHub Repo Template` folder in your REST ROBOTICS folder IS the repo. Here's how to initialize it as a Git repo:

Open **Git Bash** and navigate to the template folder:

```bash
# Change directory to the template folder
cd "C:/Users/Franklin/Documents/school onedrive copy/REST ROBOTICS/GitHub Repo Template"

# Initialize as a Git repository
git init

# Stage all the template files
git add .

# Make your first commit
git commit -m "Initial commit: Add repo structure, README files, .gitignore, and CONTRIBUTING guide"
```

---

## Part 6: Connect to GitHub and Push

Back on the GitHub page from Part 3, copy the URL for your new repo. It will look like:
`https://github.com/YOUR-USERNAME/rest-robotics.git`

Then in Git Bash:

```bash
# Connect your local folder to GitHub
git remote add origin https://github.com/YOUR-USERNAME/rest-robotics.git

# Rename the default branch to "main"
git branch -M main

# Push everything to GitHub
git push -u origin main
```

Git will ask for your GitHub username and password. For the password, use a **Personal Access Token** (not your actual password):
1. GitHub.com → Your profile → **Settings** → **Developer settings** → **Personal access tokens** → **Tokens (classic)** → **Generate new token**
2. Check the `repo` checkbox
3. Copy the token and paste it as your password in Git Bash

After this, refresh your GitHub page — you should see all the files!

---

## Part 7: Add Your Existing Arduino Code

Now copy your actual `.ino` files into the right folders. Based on your existing code:

**Autonomous team code to move:**
```
Autonomous 2025-2026/Code/Alpha/Alpha.ino
  → autonomous-team/2025-2026/milestones/alpha/alpha/alpha.ino

Autonomous 2025-2026/Code/Bravo/Bravo.ino
  → autonomous-team/2025-2026/milestones/bravo/bravo/bravo.ino

Autonomous 2025-2026/Code/Charlie/Charlie.ino
  → autonomous-team/2025-2026/milestones/charlie/charlie/charlie.ino

Autonomous 2025-2026/Code/Delta/ through Golf/
  → autonomous-team/2025-2026/competition/

Autonomous 2025-2026/Code/Tools/
  → autonomous-team/2025-2026/tools/
```

**V2X team code to move:**
```
V2X 2025-2026/Code/
  → v2x-team/2025-2026/milestones/ and competition/
```

After copying files, commit them:
```bash
git add .
git commit -m "Add Autonomous team milestone code (Alpha, Bravo, Charlie) and tools"
git push
```

---

## Part 8: The Daily Workflow

Once everything is set up, your day-to-day process is:

```bash
# 1. Before starting work — pull latest changes from teammates
git pull origin main

# 2. Create a branch for your work
git checkout -b franklin/fix-line-sensor

# 3. Write code, test on robot

# 4. Stage and commit
git add .
git commit -m "Tune PID values for line sensor on carpet surface"

# 5. Push your branch
git push origin franklin/fix-line-sensor

# 6. Go to GitHub.com → open a Pull Request → get it reviewed → merge to main
```

---

## Part 9: Claude + GitHub Integration

**What Claude can do with your GitHub repo in Cowork:**

Claude can read and write files directly in your local folder. Since your repo is cloned to your computer, Claude can:
- Read your `.ino` files and help debug code
- Suggest improvements or explain what code does
- Help you write commit messages
- Generate new code files in the right folder

**What Claude cannot do:**
- Push to GitHub on your behalf (you run the `git push` commands yourself)
- Access GitHub.com directly (it works with local files only)
- Run your Arduino code (it can't connect to hardware)

**Best workflow with Claude:**
1. Clone the repo to your computer (done above)
2. Open Cowork and make sure your `school onedrive copy` folder is connected
3. Ask Claude to "look at `autonomous-team/2025-2026/milestones/charlie/charlie.ino` and help me optimize the line following logic"
4. Claude edits the file locally
5. You review the change, then commit and push yourself

---

## Part 10: Keeping Things in Sync

**When a teammate pushes new code:**
```bash
git pull origin main
```

**If you have a conflict** (two people edited the same file):
Git will mark the conflicting lines. Open the file, pick which version to keep, then:
```bash
git add .
git commit -m "Resolve merge conflict in alpha.ino"
```

**To see what changed recently:**
```bash
git log --oneline -10
```

**To undo your last commit (before pushing):**
```bash
git reset --soft HEAD~1
```

---

## Quick Reference Card

| What you want to do | Command |
|---------------------|---------|
| Download latest from GitHub | `git pull origin main` |
| See what files changed | `git status` |
| Stage all changes | `git add .` |
| Save a snapshot | `git commit -m "your message"` |
| Send to GitHub | `git push` |
| Create a new branch | `git checkout -b branch-name` |
| Switch to a branch | `git checkout branch-name` |
| See commit history | `git log --oneline` |
| Go back to last commit | `git checkout .` |

---

## Need Help?

- GitHub Docs: https://docs.github.com
- Arduino + Git guide: https://www.arduino.cc/en/guide/git
- Ask Claude in Cowork — paste your error message and it'll help you fix it
