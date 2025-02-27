#ifndef PSIMULATIONTHREAD_H
#define PSIMULATIONTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>
#include <chrono>
#include <mutex>
#include <condition_variable>

#include <nodes/QNode>

namespace dyno
{
	class Node;
	class SceneGraph;

	class PSimulationThread : public QThread
	{
		Q_OBJECT

	public:
		static PSimulationThread* instance();

		void pause();
		void resume();
		void stop();

		void createNewScene();
		void createNewScene(std::shared_ptr<SceneGraph> scn);

		void closeCurrentScene();

		void closeAllScenes();

		void run() override;

		/**
		 * @brief Reset the simulation
		 */
		void reset(int num);

		/**
		 * @brief Continue the simulation from the current frame
		 */
		void proceed(int num);

		void startUpdatingGraphicsContext();
		void stopUpdatingGraphicsContext();

		void setTotalFrames(int num);

		int getCurrentFrameNum();
	Q_SIGNALS:
		//Note: should not be emitted from the user

		void sceneGraphChanged();

		void oneFrameFinished();
		void simulationFinished();

	public slots:
		void resetNode(std::shared_ptr<Node> node);
		void resetQtNode(Qt::QtNode& node);

		void syncNode(std::shared_ptr<Node> node);

	private:
		PSimulationThread();

		void notify();

		std::atomic<int> mTotalFrame;

		bool mReset;
	 	std::atomic<bool> mPaused;
		std::atomic<bool> mRunning;
		bool mFinished;

		std::atomic<bool> mUpdatingGraphicsContext;

		std::shared_ptr<Node> mActiveNode;

		std::chrono::milliseconds mTimeOut;
		std::timed_mutex mMutex;
		std::condition_variable_any mCond;
	};
}


#endif // PSIMULATIONTHREAD_H
