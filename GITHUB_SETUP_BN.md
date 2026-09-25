# Japan 365 POS System — GitHub আপলোড গাইড

## একটি রিপোজিটরি, দুইজন Contributor

Owner: PriyamVR  
Collaborator: borshan-karmaker  
Suggested repository: `Japan-365-POS-System`

### GitHub Desktop দিয়ে আপলোড
1. GitHub Desktop ইনস্টল করে `PriyamVR` দিয়ে Sign in করো।
2. File → New repository। Name: `Japan-365-POS-System`। Local path নির্বাচন করো। **Initialize with README বন্ধ রাখো**, কারণ এই প্রজেক্টে README আছে।
3. Create repository চাপার পর `Repository` → `Show in Explorer` দিয়ে ওই নতুন ফোল্ডার খোলো।
4. এই ZIP থেকে Extract করা ভেতরের `src`, `sql`, `tests`, `CMakeLists.txt`, `README.md`, `.gitignore` ইত্যাদি ফাইল ও ফোল্ডার নতুন repo-র ভেতরে Copy করো। পুরো মূল ফোল্ডারটিকে আবার ভেতরে কপি করবে না।
5. GitHub Desktop-এ Changes দেখা গেলে Summary: `Initial commit: Japan 365 POS System`; Commit to main চাপো।
6. Publish repository চাপো, owner `PriyamVR` নির্বাচন করো। Public/Private বেছে নিয়ে Publish করো।
7. Repo পেজে Settings → Collaborators → Add people → `borshan-karmaker` invite করো। তিনি Invite Accept করবেন।
8. সহযোগী তাঁর GitHub Desktop-এ File → Clone repository দিয়ে `PriyamVR/Japan-365-POS-System` Clone করবেন। আলাদা Branch-এ কাজ করে Push ও Pull Request করতে পারেন।
9. উভয়ের GitHub প্রোফাইলে আলাদা Repo দেখানোর প্রয়োজন হলে সহযোগী মূল Public Repo Fork করতে পারেন। Fork মূল Repo-র সঙ্গে সংযুক্ত থাকে।

### Repo-তে কী থাকবে না
- `build/`, `release/`, `.exe`, Qt DLL, `.qtcsettings/`, `*.user`, ব্যক্তিগত `.sqlite` database।
- ডেমোর জন্য সফটওয়্যারের `Insert sample data` অপশন ব্যবহার করতে পারো।

### Build
Qt Creator → Open File or Project → `CMakeLists.txt` → Desktop Qt 6 MinGW 64-bit kit → Configure → Build → Run।

### Note
এটি বিশ্ববিদ্যালয়-প্রজেক্টের স্বতন্ত্র C++/Qt POS-style implementation; Japan.365-এর official software নয়।
