#pragma once
#include "Gizmo/GizmoWidget.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"
namespace MOON
{
    namespace StateMachines
    {

        enum class OneSeekEnd
        {
            SeekFirst,
            End  // MUST be the last one
        };

        enum class TwoSeekEnd
        {
            SeekFirst,
            SeekSecond,
            End  // MUST be the last one
        };

        enum class ThreeSeekEnd
        {
            SeekFirst,
            SeekSecond,
            SeekThird,
            End  // MUST be the last one
        };

        enum class FourSeekEnd
        {
            SeekFirst,
            SeekSecond,
            SeekThird,
            SeekFourth,
            End  // MUST be the last one
        };

        enum class FiveSeekEnd
        {
            SeekFirst,
            SeekSecond,
            SeekThird,
            SeekFourth,
            SeekFifth,
            End  // MUST be the last one
        };

        enum class TwoSeekDoEnd
        {
            SeekFirst,
            SeekSecond,
            Do,
            End  // MUST be the last one
        };

    }  // namespace StateMachines
    template<typename SelectModeT>
    class StateMachine
    {
    public:
        StateMachine()
            : Mode(static_cast<SelectModeT>(0))
        {
        }
        virtual ~StateMachine()
        {
        }

    protected:
        void setState(SelectModeT mode)
        {
            Mode = mode;
            onModeChanged();
        }

        void ensureState(SelectModeT mode)
        {
            if (Mode != mode) {
                Mode = mode;
                onModeChanged();
            }
        }

        /** Ensure the state machine is the provided mode
         * but only if the mode is an earlier state.
         *
         * This allows one to return to previous states (e.g.
         * for modification), only if that state has previously
         * been completed.
         */
        void ensureStateIfEarlier(SelectModeT mode)
        {
            if (Mode != mode) {
                if (mode < Mode) {
                    Mode = mode;
                    onModeChanged();
                }
            }
        }

        SelectModeT state() const
        {
            return Mode;
        }

        bool isState(SelectModeT state) const
        {
            return Mode == state;
        }

        void setNextState(std::optional<SelectModeT> nextState)
        {
            nextMode = nextState;
        }

        std::optional<SelectModeT> getNextState()
        {
            return nextMode;
        }

        void applyNextState()
        {
            if (nextMode) {
                auto next = std::move(*nextMode);
                nextMode = std::nullopt;
                setState(next);
            }
        }

        bool isFirstState() const
        {
            return Mode == (static_cast<SelectModeT>(0));
        }

        bool isLastState() const
        {
            return Mode == SelectModeT::End;
        }

        constexpr SelectModeT getFirstState() const
        {
            return static_cast<SelectModeT>(0);
        }

        SelectModeT computeNextMode() const
        {
            auto modeint = static_cast<int>(state());

            if (modeint < maxMode) {
                auto newmode = static_cast<SelectModeT>(modeint + 1);
                return newmode;
            }
            else {
                return SelectModeT::End;
            }
        }

        void moveToNextMode()
        {
            setState(computeNextMode());
        }

        void reset()
        {
            nextMode = std::nullopt;
            if (Mode != static_cast<SelectModeT>(0)) {
                setState(static_cast<SelectModeT>(0));
            }
        }

        virtual bool onModeChanged()
        {
            return true;
        };

    private:
        SelectModeT Mode;
        std::optional<SelectModeT> nextMode;
        static const constexpr int maxMode = static_cast<int>(SelectModeT::End);
    };
    namespace ConstructionMethods
    {

        enum class DefaultConstructionMethod
        {
            End  // Must be the last one
        };

    }  // namespace ConstructionMethods

    template<typename ConstructionMethodT>
    class ConstructionMethodMachine
    {
    public:
        ConstructionMethodMachine(
            ConstructionMethodT constructionmethod = static_cast<ConstructionMethodT>(0)
        )
            : ConstructionMode(constructionmethod)
        {
        }
        virtual ~ConstructionMethodMachine()
        {
        }

    protected:
        void setConstructionMethod(ConstructionMethodT mode)
        {
            ConstructionMode = mode;
            onConstructionMethodChanged();
        }

        ConstructionMethodT constructionMethod() const
        {
            return ConstructionMode;
        }

        bool isConstructionMethod(ConstructionMethodT state) const
        {
            return ConstructionMode == state;
        }

        void resetConstructionMode()
        {
            ConstructionMode = static_cast<ConstructionMethodT>(0);
        }

        void initConstructionMethod(ConstructionMethodT mode)
        {
            ConstructionMode = mode;
        }

        // Cyclically iterate construction methods
        ConstructionMethodT getNextMethod() const
        {
            auto modeint = static_cast<int>(ConstructionMode);


            if (modeint < (maxMode - 1)) {
                auto newmode = static_cast<ConstructionMethodT>(modeint + 1);
                return newmode;
            }
            else {
                return static_cast<ConstructionMethodT>(0);
            }
        }

        void iterateToNextConstructionMethod()
        {
            if (ConstructionMethodsCount() > 1) {
                setConstructionMethod(getNextMethod());
            }
        }

        virtual void onConstructionMethodChanged() {};

        static constexpr int ConstructionMethodsCount()
        {
            return maxMode;
        }

    private:
        ConstructionMethodT ConstructionMode;
        static const constexpr int maxMode = static_cast<int>(ConstructionMethodT::End);
    };
    template<
        typename HandlerT,            // A type for which the handler template is instantiated
        typename SelectModeT,         // The state machine defining the states that the handle iterates
        int PInitAutoConstraintSize,  // The initial size of the AutoConstraint>
        typename ConstructionMethodT = ConstructionMethods::DefaultConstructionMethod>
    class DrawSketchDefaultHandler : public GizmoWidget,
        public StateMachine<SelectModeT>,
        public ConstructionMethodMachine<ConstructionMethodT> {
    public:
        using DSDH = DrawSketchDefaultHandler<HandlerT, SelectModeT, PInitAutoConstraintSize, ConstructionMethodT>;
        DrawSketchDefaultHandler(const std::string&name,
            ConstructionMethodT constructionmethod = static_cast<ConstructionMethodT>(0)
        )
            : GizmoWidget(name),
            StateMachine<SelectModeT>(),
            ConstructionMethodMachine<ConstructionMethodT>(constructionmethod),
            continuousMode(true)
        {
            // Define widget events
            this->CallbackMapper->SetCallbackMethod(ExecuteCommand::LeftButtonPressEvent, GizmoEvent::NoModifier, 0,
                0, 0, WidgetEvent::Select, this, DSDH::MousePressed);
            this->CallbackMapper->SetCallbackMethod(ExecuteCommand::MouseMoveEvent, GizmoEvent::NoModifier, 0,
                0, 0, WidgetEvent::Move3D, this, DSDH::MouseMove);
        }
        ~DrawSketchDefaultHandler() override
        {
        }
        virtual void updateDataAndDrawToPosition(Vec2 onSketchPos)
        {

        }

        static void MousePressed(AbstractWidget*w) {
            DSDH* self = reinterpret_cast<DSDH*>(w);
            int mousePos=Interactor->GetEventPosition();;
            self->onButtonPressed(Vec2(mousePos[0], mousePos[1]));
        }
        static void MouseMove(AbstractWidget*w) {
            DSDH* self = reinterpret_cast<DSDH*>(w);
            int mousePos = Interactor->GetEventPosition();;
            self->mouseMove(Vec2(mousePos[0], mousePos[1]));

        }
        void mouseMove(Vec2 snapHandle)
        {
            updateDataAndDrawToPosition(snapHandle);
        }
        virtual void onButtonPressed(Vec2 onSketchPos)
        {
            this->updateDataAndDrawToPosition(onSketchPos);
            if (canGoToNextMode()) {
                this->moveToNextMode();
            }
        }

        virtual bool canGoToNextMode()
        {
            return true;
        }
    protected:
        bool continuousMode;
    };
}