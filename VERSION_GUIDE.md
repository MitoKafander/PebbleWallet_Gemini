# GeminiWallet Version Control Guide

You now have a solid, working version of your app. Here is how we manage versions so you never lose a working state.

## 1. What I just did: "The v1.0 Milestone"
I have created a **Git Tag** called `v1.0` and pushed it to GitHub. 
- A tag is like a "snapshot in time." 
- No matter what we change in the future, this exact code is now permanently locked under the name `v1.0`.
- You can see it on GitHub under the **"Releases"** or **"Tags"** section.

## 2. How to "Revert" if something breaks
If we start making "Quality of Life" changes and the app stops working, you can force the Rebble Cloud environment to go back to this safe version by running this in the terminal:

```bash
git fetch --tags
git reset --hard v1.0
pebble clean && pebble build
```

## 3. Best Practice: "Save Points"
Before we start a big new feature, we can do one of two things:

### Option A: Create a new Tag (Simple)
If you like the current state, we just make a new tag:
`git tag -a v1.1 -m "Added brightness toggle"`

### Option B: Use Branches (Advanced)
We can create a "Experimental" branch. We work there, and if it's good, we merge it into the "Main" branch. If it's bad, we just delete the branch and your "Main" code stays perfect.

## 4. GitHub Releases
On the GitHub website, you can go to **Releases -> Create a new release**, select the `v1.0` tag I just made, and **upload your `GeminiWallet.pbw` file** there.
- This gives you a permanent backup of the actual "installer" file for this specific version.

---

**Do you want to start on a new "Quality of Life" feature now, knowing your v1.0 is safe?**
(e.g., The "Invert Colors" or "Max Brightness" features we discussed).
