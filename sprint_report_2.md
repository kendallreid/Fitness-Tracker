# Sprint 2 Report
Video Link:
## What's New (User Facing)
* Exercise Tracking
* Calorie/Meal Tracking
* Goal Tracking
* Updated UI, consistent between tabs
## Work Summary (Developer Facing)
In this sprint we focused on implementing the core functionality of most fitness apps, which is the trackers. We have implemented our workout/meal/goal trackers, and are currently working on the sleep tracker. These features all support creation, deletion, and editing and have tables in our database to show this. One thing that required a bit more planning across our issues was the aspect of how we obtain a user id to make sure that the correct features updated per user. The request bodies that were returned did not typically have the user id attached, so we decided to deploy a cookie that was created on login which we could then extract the id from. This made of code cleaner and fixed the functionality issues that we were having. Lastly, we decided on a clean UI for our tabs that was easier to work with the the previous. 
## Unfinished Work
The unfinished work for this sprint is the sleep tracker. It just ended up being more work than we anticipated, although it is mostly completed. The plan is to have it ready in the early stages of sprint 3. Luckily, our sprint 3 tasks are not directly correlated with this, but rather just something that can be tailored to accomodate it once it's done.
## Completed Issues/User Stories
Here are links to the issues that we completed in this sprint:
* [URL of issue 1](https://github.com/kendallreid/Fitness-Tracker/issues/1)
* [URL of issue 2](https://github.com/kendallreid/Fitness-Tracker/issues/7)
* [URL of issue 3](https://github.com/kendallreid/Fitness-Tracker/issues/8)
* [URL of issue 4](https://github.com/kendallreid/Fitness-Tracker/issues/20)

## Incomplete Issues/User Stories
Here are links to issues we worked on but did not complete in this sprint:
* There were no issues that were started and not completed in sprint 2

## Code Files for Review
Please review the following code files, which were actively developed during this
sprint, for quality:
* [CalorieTracker.html](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/frontend/CalorieTracker.html)
* [completedGoals.html](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/frontend/completedGoals.html)
* [goalTracker.html](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/frontend/goalTracker.html)
* [sessions.html](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/frontend/sessions.html)
* [exercise.html](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/frontend/exercise.html)
* [goalTracker.h](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/goalTracker.h)
* [goalTracker.h](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/routes/goalTracker.h)
* [goalTracker.cpp](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/routes/goalTracker.cpp)
* [goalTracker.cpp](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/goalTracker.cpp)
* [calorie_tracker.h](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/routes/calorie_tracker.h)
* [calorie_tracker.cpp](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/routes/calorie_tracker.cpp)
* [exercise.h](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/routes/exercise.h)
* [exercise.cpp](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/routes/exercise.cpp)
* [session.h](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/routes/session.h)
* [session.cpp](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/routes/session.cpp)
* [goals.cpp](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/goals.cpp)
* [main.cpp](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/main.cpp)
* [helper.h](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/helper.h)
* [schema.h](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/db/schema.h)
* [schema.cpp](https://github.com/kendallreid/Fitness-Tracker/blob/main/code/backend/db/schema.cpp)

## Retrospective Summary
Here's what went well:
* Item 1: We were able to get most of our work done on time
* Item 2: We improved our efficiency by ensuring that each person only had 1 issue to work on

Here's what we'd like to improve:
* Item 1: I think that we can make better use of the time that we have. Starting issues as soon as possible ensures that we can maximize our progress

Here are changes we plan to implement in the next sprint:
* Item 1: Have a home page, email authentication, and forget password feature
* Item 2: A leaderboard, and friend adding feature
* Item 3: Scoring rules applied
* Item 4: Personalization (If there is time)
