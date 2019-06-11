package caoss.simulator.os.scheduling;


import java.util.Queue;
import java.util.concurrent.LinkedBlockingQueue;

import caoss.simulator.Program;
import caoss.simulator.configuration.Hardware;
import caoss.simulator.hardware.DeviceId;
import caoss.simulator.hardware.Timer;
import caoss.simulator.os.Dispatcher;
import caoss.simulator.os.Logger;
import caoss.simulator.os.ProcessControlBlock;
import caoss.simulator.os.ProcessControlBlock.State;
import caoss.simulator.os.Scheduler;


public class FSOScheduler implements Scheduler<SchedulingState> {

	private static final int QUANTUM = 10;
	private static final int QUOTA = 20;
	private static final int PERIOD = 150;
	private static final int PRIOLEVELS = 4;


	private final Queue<ProcessControlBlock<SchedulingState>>[] readyQueues;
	private final Queue<ProcessControlBlock<SchedulingState>> blockedQueue;
	private ProcessControlBlock<SchedulingState> running; // just one CPU
			
	private final Timer timer = (Timer) Hardware.devices.get(DeviceId.TIMER);

	private long lastUpgradeTime = 0;  // last priority upgrade time
	
	
	@SuppressWarnings("unchecked")
	public FSOScheduler( ) {
		this.readyQueues = (Queue<ProcessControlBlock<SchedulingState>>[]) new LinkedBlockingQueue[PRIOLEVELS];
		this.blockedQueue = new LinkedBlockingQueue<ProcessControlBlock<SchedulingState>>();
		
		for (int i = 0; i < PRIOLEVELS; i++) {
			this.readyQueues[i] = new LinkedBlockingQueue<ProcessControlBlock<SchedulingState>>();
		}
		
		this.lastUpgradeTime = Hardware.clock.getTime(); 
	}
	
	
	@Override
	public synchronized void newProcess(Program program) {	
	
		ProcessControlBlock<SchedulingState> pcb =
				new ProcessControlBlock<SchedulingState>(program, 
						new SchedulingState(0, QUOTA));
		Logger.info("Create process " + pcb.pid + " to run program " + program.getFileName());
		
		// TODO:			
		if(running == null)
			dispatch(pcb);
		else
			this.readyQueues[0].add(pcb);
			
		upgrade();
		logQueues();
	}

	@Override
	public synchronized void ioRequest(ProcessControlBlock<SchedulingState> pcb) {
		Logger.info("Process " + pcb.pid + ": IO request");
		
		// TODO:
		
		pcb.getSchedulingState().setQuota((int) (pcb.getSchedulingState().getQuota() - (Hardware.clock.getTime() - pcb.getSchedulingState().getSchedulingTime())));
		if(pcb.getSchedulingState().getQuota() <= 0) {
			Logger.info("Process " + pcb.pid + ": quota expired");
			if(pcb.getSchedulingState().getLevel() < 3)
				pcb.getSchedulingState().setLevel(pcb.getSchedulingState().getLevel() + 1);
			pcb.getSchedulingState().setQuota(QUOTA);	
		}
		
		readyQueues[pcb.getSchedulingState().getLevel()].remove(pcb);
		
		this.blockedQueue.add(pcb);
			
		chooseNext();	
		
		upgrade();
		logQueues();
	}

	@Override
	public synchronized void ioConcluded(ProcessControlBlock<SchedulingState> pcb) {
		Logger.info("Process " + pcb.pid + ": IO concluded");
		
		// TODO:	
		this.blockedQueue.remove(pcb);
		
		if(running == null)
			dispatch(pcb);
		else 
			this.readyQueues[pcb.getSchedulingState().getLevel()].add(pcb);
		
			
		upgrade();
		logQueues();
	}

	@Override
	public synchronized void quantumExpired(ProcessControlBlock<SchedulingState> pcb) {
		Logger.info("Process " + pcb.pid + ": quantum expired");
		
		// TODO:	
		
		pcb.getSchedulingState().setQuota(pcb.getSchedulingState().getQuota() - QUANTUM);
		if(pcb.getSchedulingState().getQuota() <= 0) {
			Logger.info("Process " + pcb.pid + ": quota expired");
			if(pcb.getSchedulingState().getLevel() < 3)
				pcb.getSchedulingState().setLevel(pcb.getSchedulingState().getLevel() + 1);
			pcb.getSchedulingState().setQuota(QUOTA);
		}
		
		readyQueues[pcb.getSchedulingState().getLevel()].add(pcb);
		
		chooseNext();	
		
		upgrade();
		logQueues();
	}

	@Override
	public synchronized void processConcluded(ProcessControlBlock<SchedulingState> pcb) {
		Logger.info("Process " + pcb.pid + ": execution concluded");
		
		long turnarround = Hardware.clock.getTime() - pcb.arrivalTime;
		Logger.info("Process " + pcb.pid + ": turnarround time: " + turnarround);	
			
		chooseNext();
		
		upgrade();
		logQueues();
	}
	


	private void upgrade() {
		long currentTime = Hardware.clock.getTime();
		long elapsed = currentTime - this.lastUpgradeTime;
		if (elapsed >= PERIOD) {
			Logger.info("Upgrade priorities");	
			
			
			// TODO:
			for(ProcessControlBlock<SchedulingState> pcb : this.readyQueues[0])
				pcb.getSchedulingState().setQuota(QUOTA);
			
			for (ProcessControlBlock<SchedulingState> pcb : this.readyQueues[1]) {
				pcb.getSchedulingState().setQuota(QUOTA);
				pcb.getSchedulingState().setLevel(0);
				this.readyQueues[0].add(pcb);
				this.readyQueues[1].remove(pcb);
			}
			
			for (ProcessControlBlock<SchedulingState> pcb : this.readyQueues[2]) {
				pcb.getSchedulingState().setQuota(QUOTA);
				pcb.getSchedulingState().setLevel(0);
				this.readyQueues[0].add(pcb);
				this.readyQueues[2].remove(pcb);
			}
			
			for (ProcessControlBlock<SchedulingState> pcb : this.readyQueues[3]) {
				pcb.getSchedulingState().setQuota(QUOTA);
				pcb.getSchedulingState().setLevel(1);
				this.readyQueues[1].add(pcb);
				this.readyQueues[3].remove(pcb);
			}
			
			for(ProcessControlBlock<SchedulingState> pcb : blockedQueue) {
				int blockedLevel = pcb.getSchedulingState().getLevel();
				if(blockedLevel == 0) {
					pcb.getSchedulingState().setQuota(QUOTA);
				} else if(blockedLevel == 1 || blockedLevel == 2) {
					pcb.getSchedulingState().setQuota(QUOTA);
					pcb.getSchedulingState().setLevel(0);
				}else {
					pcb.getSchedulingState().setQuota(QUOTA);
					pcb.getSchedulingState().setLevel(1);
					
				}
			}
			
			if(running != null) {
				int runningLevel = running.getSchedulingState().getLevel();
				if(runningLevel == 0) {
					running.getSchedulingState().setQuota(QUOTA);
				} else if(runningLevel == 1 || runningLevel == 2) {
					running.getSchedulingState().setQuota(QUOTA);
					running.getSchedulingState().setLevel(0);
				} else {
					running.getSchedulingState().setQuota(QUOTA);
					running.getSchedulingState().setLevel(1);
				}
			}
				
			
			
			this.lastUpgradeTime = currentTime;
		}
			
	}
	
	
	private void chooseNext() {
		for (Queue<ProcessControlBlock<SchedulingState>> queue : this.readyQueues) {
			if (queue.size() > 0) {
				dispatch(queue.poll());
				return;
			}
		}
		
		dispatch(null);
	}
	
	private void dispatch(ProcessControlBlock<SchedulingState> pcb) {
		Dispatcher.dispatch(pcb);
		running = pcb;
		
		if (pcb != null) {
			SchedulingState state = pcb.getSchedulingState();
			state.setSchedulingTime(Hardware.clock.getTime());
			timer.set(QUANTUM);
			Logger.info("Run process "+ pcb.pid +" (quantum="+ QUANTUM +", quota="+ state.getQuota()+")");
		}
		else
			timer.set(0);
	}
	
	
	private void logQueues() {
		int i = 0;
		for (Queue<ProcessControlBlock<SchedulingState>> queue : this.readyQueues) {
			Logger.info("Queue " + i + ": " + queue);
			i++;
		}
		Logger.info("Blocked " + blockedQueue);
	}
}
