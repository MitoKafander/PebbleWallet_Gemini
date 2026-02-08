# How to Sync Gemini Wallet to the Web (Rebble Cloud)

Since the Pebble SDK is hard to run locally, the best way to develop is using **Rebble Cloud** (browser-based). Here is how to get this local code into the web seamlessly:

### Step 1: Create a GitHub Repository
1. Go to [github.com/new](https://github.com/new).
2. Name it `GeminiWallet`.
3. Keep it **Public** (or Private if you have GitHub linked to Rebble).
4. Do **NOT** initialize with a README or License.
5. Click **Create repository**.

### Step 2: Link your local code to GitHub
Run these commands in your terminal (or just ask me to run them once you have the URL!):

```bash
git remote add origin https://github.com/YOUR_USERNAME/GeminiWallet.git
git branch -M main
git push -u origin main
```

### Step 3: Open in Rebble Cloud
1. Go to [cloud.repebble.com](https://cloud.repebble.com).
2. Click **Import Project** or **New Project**.
3. Choose **Import from GitHub**.
4. Select your `GeminiWallet` repository.

### Step 4: Syncing Changes
Every time I make a change for you locally, you just need to run:
```bash
git add .
git commit -m "update"
git push
```
Then, in Rebble Cloud, just click **Pull** (or the Git icon) to refresh the files. No more copy-pasting!
