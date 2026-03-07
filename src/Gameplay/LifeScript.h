#pragma once
#include <ECS_Engine.h>
#include "ShootingScript.h"

START_SCRIPT(PointScript)
private:
	int m_pPoint = 0;
	Entity* m_pPointText = nullptr;
	std::string m_pText;
public:
void OnStart()
{
	m_pPoint = 0;
	m_pPointText = Ui::Text(0.5f, 0.02f, "Point : ");
}

void OnUpdate()
{
	m_pText = "Point : " + std::to_string(m_pPoint);
	UiTextComponent* textComp = m_pPointText->GetComponent<UiTextComponent>();

	if (textComp) {
		textComp->m_text = m_pText;
	}
}

void AddPoint(int _point) {
	m_pPoint += _point;
}

void AddPoint() {
	m_pPoint += 10;
}

END_SCRIPT(PointScript)

START_SCRIPT(LifeScript)

private : 
	float m_invincibility_timer = 0.05f;
	int m_life = 100;
	Entity* m_pLifeText = nullptr;
public :
	std::string m_pText;
void OnStart()
{
	if (owner->GetType() == Entity::TYPE::Player) m_pLifeText = Ui::Text(0.9f, 0.02f, "Life : ");
}

void OnUpdate()
{
	m_invincibility_timer -= ECS_APP->GetTimer().DeltaTime();
	if (owner->GetType() == Entity::TYPE::Player) {
		m_pText = "Life : " + std::to_string(m_life);
		UiTextComponent* textComp = m_pLifeText->GetComponent<UiTextComponent>();

		if (textComp) {
			textComp->m_text = m_pText;
		}
	}

	if (m_life <= 0) {
		ECS_ECS->DestroyEntity(owner);
	}
}

void OnDestroy()
{
	m_life = 0;
}

void TakeDamage( int damage){
	m_life -= damage;
	std::cout << "life : " << m_life << std::endl;
}

void TakeDamage() {
	m_life -= 10.f;
}

void Heal(int Heal) {
	m_life += Heal;
	std::cout << "life : " << m_life << std::endl;
}

void Heal() {
	m_life += 10.f;
}

void SetLife(int life) {
	m_life = life;
}

void OnCollisionEnter(Entity* other) {
	if (other->GetType() != Entity::TYPE::Bullet && other->GetType() != Entity::TYPE::Ennemy) return;
	if (m_invincibility_timer > 0.f) return;

	if (other->GetType() == Entity::TYPE::Bullet)
	{
		ScriptComponent* sc = other->GetComponent<ScriptComponent>();
		if (!sc || !sc->m_instance) return;
		BulletScript* bs = dynamic_cast<BulletScript*>(sc->m_instance);
		if (!bs) return;
		if (owner->GetId() == bs->shooter->GetId()) return;

		m_life -= bs->damage;
		m_invincibility_timer = 0.05f;

		std::vector<ScriptComponent*> shooterScripts = bs->shooter->GetComponents<ScriptComponent>();
		for (ScriptComponent* script : shooterScripts) {
			PointScript* ps = dynamic_cast<PointScript*>(script->m_instance);
			if (ps) { ps->AddPoint(); break; }
		}
	}
}

END_SCRIPT(LifeScript)
