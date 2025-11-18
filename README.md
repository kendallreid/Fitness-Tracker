# Fitness Tracker
WSU CPTS322 Project

Kendall Reid
Ethan Karasha
Zachary Marseglia
Alba Raya Sanchez

A full-stack fitness tracker web app to track calories, exercises, sleep, make and complete goals, all while competing with other users.

## Table of Contents

* [Features](#features)
* [Tech Stack](#tech-stack)
* [Installation](#installation)
* [Usage](#usage)
* [Project Structure](#project-structure)
* [Database Schema](#database-schema)

---

## Features

* Track daily calories in/out and net calories
* Record exercises and sessions
* Maintain streaks and points
* Display leaderboard of top users
* View personal profile and weekly logs
* Manage goals and completed goals

---

## Tech Stack

* **Backend:** C++, Crow C++ web framework, SQLite
* **Frontend:** HTML, CSS, JavaScript
* **Libraries:** libsodium (for password hashing), SQLite3
* **Build System:** CMake

---

## Installation

1. **Clone the repository**

```bash
git clone https://github.com/kendallreid/fitness-tracker.git
cd fitness-tracker
```

2. **Install dependencies**

   * Crow: included in `external/crow`
   * libsodium: via your package manager
   * SQLite3: via your package manager
   * CMake >= 3.10

3. **Build**

```bash
mkdir build
cd build
cmake ..
make
```

4. **Run**

```bash
cd ..
./build/fitness
```

Then open [http://localhost:8080](http://localhost:8080) in your browser.

---

## Usage

* Navigate to `/` for registration.
* `/auth/login` for login.
* `/home` for the dashboard.
* `/leaderboard.html` to view the top users.
* Track calories, exercises, and goals via their respective pages.

---

## Project Structure

```
code/
├─ backend/
|  ├─ db/
│       └─ schema.h  
│  ├─ routes/          # Route handlers (login, registration, exercises, sessions, goals, leaderboard)
│  ├─ LogIn.h
│  ├─ leaderboard.h
│  ├─ helper.h
│  └─ main.cpp         # Server entry point
├─ frontend/
│  ├─ login.html
│  ├─ UserRegistration.html
│  ├─ HomePage.html
│  ├─ leaderboard.html
│  ├─ exercise.html
│  ├─ goalTracker.html
│  ├─ sessions.html
│  └─ ... other pages
external/
├─ crow/                # Crow C++ web framework
docs/
CMakeLists.txt
README.md
```

---

## Database Schema

**Users Table**

| Column        | Type    | Notes       |
| ------------- | ------- | ----------- |
| id            | INTEGER | Primary key |
| first_name    | TEXT    |             |
| last_name     | TEXT    |             |
| username      | TEXT    | Unique      |
| password_hash | TEXT    |             |
| email         | TEXT    |             |
| score         | INTEGER | Default 0   |

Other tables include `workouts`, `goals`, `nutrition`, `sessions`, `user_goals`, `exercises`, `goal_progress`

---
