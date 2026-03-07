#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include "GameTimer.h"
#include <string>

struct State {
public:
	std::function<void()> onEnter;
	std::function<void()> onUpdate;
	std::function<void()> onExit; 

	std::function<bool()> Transition;

	std::string name;

protected:
	int stateNumber;
	void* self = this;

	template<typename> friend class StateMachine;
};

template<typename T>
class StateMachine {
	
	float stateTime;
	float GlobalTime;

	State globalState;
	int pending = -1;
	int currentState = -1;

	std::vector<State> states;
	T* owner;
public :
	StateMachine(T* own) { owner = own; }
	virtual ~StateMachine() = default;

	void ToState(State state);
	void ToState(std::string NameState);

	std::vector<State> GetStates() { return states; }

	void Update();
	void CheckTransition();

	template <typename S>
	void AddState();

	void AddState(std::function<void()> onEnter, std::function<void()> onUpdate, std::function<void()> onExit, std::function<bool()> Trans, std::string name);

	void AddState(State* state);

	template <typename S>
	void SetOriginalState();
};

/////////////////////////////////////
/// @brief Add a state to an owner with an struct.
/// @temp use a struct with function inside
/////////////////////////////////////
template<typename T>
template<typename S>
void StateMachine<T>::AddState()
{
	static S state;
	State Newstate;

	Newstate.self = &state;

	Newstate.onEnter = [p = &state]() { reinterpret_cast<S*>(p)->onEnter(); };
	Newstate.onUpdate = [p = &state]() { reinterpret_cast<S*>(p)->onUpdate(); };
	Newstate.onExit = [p = &state]() { reinterpret_cast<S*>(p)->onExit(); };
	Newstate.Transition = [p = &state]() { return reinterpret_cast<S*>(p)->Transition(); };
	Newstate.stateNumber = states.size();
	Newstate.name = typeid(S).name();

	if constexpr (requires (S * s, T * o) { s->owner = o; }) {
		state.owner = owner;
	}

	states.push_back(Newstate);
}

/////////////////////////////////////
/// @brief Add a state.
/// @param the onEnter function of the state
/// @param the onUpdate function of the state
/// @param the onExitFunction of the state
/// @param the trans to go to the state
/// @param the name of the state
/////////////////////////////////////
template<typename T>
inline void StateMachine<T>::AddState(std::function<void()> onEnter, std::function<void()> onUpdate, std::function<void()> onExit, std::function<bool()> Trans,std::string name)
{
	State* state = new State();

	state->onEnter = onEnter;
	state->onUpdate = onUpdate;
	state->onExit = onExit;
	state->Transition = Trans;
	state->stateNumber = states.size();
	state->name = name;
	states.push_back(*state);
}

/////////////////////////////////////
/// @brief Add a script with the state struct( the exact struct than on the top of that page.
/// @param the state struct you've created
/////////////////////////////////////
template<typename T>
void StateMachine<T>::AddState(State* state) {
	State* NewState = new State();

	NewState = state;
	NewState->stateNumber = states.size();
	states.push_back(*state);
}

/////////////////////////////////////
/// @brief update each state.
/////////////////////////////////////
template<typename T>
void StateMachine<T>::Update()
{
	stateTime += 0.16f;
	GlobalTime += 0.16f;
	CheckTransition();
	if (currentState != pending && pending != -1) {
		if (currentState != -1 && states[currentState].onExit) {
			states[currentState].onExit();
		}
		currentState = pending;
		stateTime = 0.0f;
		if (currentState != -1 && states[currentState].onEnter) {
			states[currentState].onEnter();
		}
	}
	if (currentState != -1 && currentState < static_cast<int>(states.size())) {
		if (states[currentState].onUpdate) states[currentState].onUpdate();
	}
}

template<typename T>
void StateMachine<T>::CheckTransition() {
	for (State& state : states) {
		if (state.Transition && state.Transition()){
			ToState(state); 
			break; 
		} 
	}
	
}

/////////////////////////////////////
/// @brief switch to a state.
/// @param take the whole state you want
/////////////////////////////////////
template<typename T>
void StateMachine<T>::ToState(State state)
{ 
	pending = state.stateNumber;
} 

/////////////////////////////////////
/// @brief switch to a state.
/// @param take the name of the state
/////////////////////////////////////
template<typename T>
void StateMachine<T>::ToState(std::string NameState)
{
	for (size_t i = 0; i < states.size(); ++i) {
		if (states[i].name == NameState) {
			pending = static_cast<int>(i);
			break;
		}
	}
}

/////////////////////////////////////
/// @brief set the base state of the owner, the one that it will start with.
/////////////////////////////////////
template <typename T>
template <typename S>
void StateMachine<T>::SetOriginalState()
{
	static S state;
	State Newstate;

	Newstate.self = &state;

	Newstate.onEnter = [p = &state]() { reinterpret_cast<S*>(p)->onEnter(); };
	Newstate.onUpdate = [p = &state]() { reinterpret_cast<S*>(p)->onUpdate(); };
	Newstate.onExit = [p = &state]() { reinterpret_cast<S*>(p)->onExit(); };
	Newstate.Transition = [p = &state]() { return reinterpret_cast<S*>(p)->Transition(); };
	Newstate.stateNumber = states.size();
	Newstate.name = typeid(S).name();

	states.push_back(Newstate);
}
