#pragma once
#include "Interactive/Widgets/DrawSketchHandler.h"
#include "Interactive/Interactive/RenderWindowInteractor.h"
#include "Interactive/Interactive/Event.h"
#include "Interactive/Interactive/ExecuteCommand.h"
#include "Interactive/Interactive/WidgetCallbackMapper.h"
#include "Interactive/Interactive/WidgetEvent.h"
#include "renderer/SceneView.h"
#include "core/component/CGeometryLine.h"
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include "Geometry.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "core/log.h"

#include <Core/ECS/Actor.h>
#include <type_traits>
#include <optional>
#include <Eigen/Core>
namespace MOON
{
    template<typename T>
    auto toPointerVector(const std::vector<std::unique_ptr<T>>& vector)
    {
        std::vector<T*> vp(vector.size());

        std::transform(vector.begin(), vector.end(), vp.begin(), [](auto& p) { return p.get(); });

        return vp;
    }
    using Vec2 = Eigen::Vector2f;
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
    class DrawSketchDefaultHandler : public DrawSketchHandler,
        public StateMachine<SelectModeT>,
        public ConstructionMethodMachine<ConstructionMethodT> {
    public:
        using DSDH = DrawSketchDefaultHandler<HandlerT, SelectModeT, PInitAutoConstraintSize, ConstructionMethodT>;
        DrawSketchDefaultHandler(const std::string&name,
            ConstructionMethodT constructionmethod = static_cast<ConstructionMethodT>(0)
        )
            : DrawSketchHandler(name),
            StateMachine<SelectModeT>(),
            ConstructionMethodMachine<ConstructionMethodT>(constructionmethod),
            continuousMode(true)
        {            

            setActive(false);
            makePlane(SketcherPlane2D());
        }
        ~DrawSketchDefaultHandler() override
        {
        }

        virtual void onReset()
        {
        }
        void reset()
        { 
            clearEdit();
            //for (auto& ac : sugConstraints) {
            //    ac.clear();
            //}

            //AutoConstraints.clear();
            ShapeGeometry.clear();
            //ShapeConstraints.clear();

            onReset();

            ModeStateMachine::reset();

            //applyCursor();
        }
        bool handleContinuousMode()
        {

            if (continuousMode) {
                // This code enables the continuous creation mode.
                reset();
                // It is ok not to call to purgeHandler in continuous creation mode because the
                // handler is destroyed by the quit() method on pressing the right button of the mouse
                return false;
            }
            else {
                //sketchgui->purgeHandler();  // no code after, Handler get deleted in ViewProvider
                return true;
            }
        }
        virtual void executeCommands() {
            CORE_INFO("add {0} geometry to sketcher obj ", ShapeGeometry.size());

            //auto& view = GetService(::Editor::Panels::SceneView);
            //auto scene = view.GetScene();
            for (auto& geo : ShapeGeometry) {
                //auto& actor = scene->CreateActor("", "SketchGeomertyLine");
                //auto& geoComp = actor.AddComponent<Core::ECS::Components::CGeometryLine>();
                //actor.AddComponent<Core::ECS::Components::CModelRenderer>();
                //actor.AddComponent<Core::ECS::Components::CMaterialRenderer>();
                //geoComp.setGeometry(geo.get());
                //geoComp.discretizationShape(plane);
                SketcherObj* sketchobj=SketcherObjManager::instance().GetCurrentActiveSketcherObj();
                if (sketchobj) {
                    sketchobj->addGeometry((geo));
                }
            }
        }
        bool finish()
        {
            if (this->isState(SelectMode::End)) {
                //unsetCursor();
                //resetPositionText();

                //try {
                    executeCommands();

                //    if (sugConstraints.size() > 0) {
                //        beforeCreateAutoConstraints();

                //        generateAutoConstraints();

                //        createAutoConstraints();
                //    }
                //}
                //catch (const Base::RuntimeError& e) {
                //    // RuntimeError exceptions inside of the block above must provide a translatable
                //    // message. It is reported both to developer (report view) and user (notifications
                //    // area).
                //    Base::Console().error(e.what());
                //}

                //// Keep the recompute separate so that everything is drawn even if execution fails
                //// partially
                //try {
                //    tryAutoRecomputeIfNotSolve(sketchgui->getSketchObject());
                //}
                //catch (const Base::RuntimeError& e) {
                //    // RuntimeError exceptions inside of the block above must provide a translatable
                //    // message. It is reported both to developer (report view) and user (notifications
                //    // area).
                //    Base::Console().error(e.what());
                //}
                //return handleContinuousMode();
                return handleContinuousMode();
            }
            return false;
        }
        virtual void updateDataAndDrawToPosition(Base::Vector2d onSketchPos)
        {

        }
        virtual void rightButtonOrEsc() {
            if (this->isFirstState()) {
                quit();
            }
            else
            {
                handleContinuousMode();
            }
        }
        virtual void onLeftMousePressed()override {
            ButtonPressParse();
        }
        virtual void onLeftMouseReleased()override {
            ButtonReleaseParse();
        }
        virtual void onMouseMove()override {
            DrawSketchHandler::onMouseMove();
            MouseMoveParse();
        }
        virtual void onRightMousePressed()override {
            rightButtonOrEsc();
        }

        void ButtonPressParse() {
            onButtonPressed(onSketchPos);
        }
        void ButtonReleaseParse() {
            releaseButton(onSketchPos);
        }
        void MouseMoveParse() {
            mouseMove(onSketchPos);
        }
        virtual void mouseMove(Base::Vector2d pos)
        {
            updateDataAndDrawToPosition(pos);
        }
        virtual void onButtonPressed(Base::Vector2d pos)
        {
            this->updateDataAndDrawToPosition(pos);
            if (canGoToNextMode()) {
                this->moveToNextMode();
            }
        }
        virtual void onKeyPress(const std::string& key) {
            if (key=="M" && !this->isLastState()) {
                this->iterateToNextConstructionMethod();
            }
        }
        virtual bool releaseButton(Base::Vector2d onSketchPos)
        {
            if (finish()) {

            }
            return true;
        }
        virtual bool canGoToNextMode()
        {
            return true;
        }
        virtual void createShape(bool onlyeditoutline)
        {
            
        }
        void CreateAndDrawShapeGeometry()
        {
            clearEdit();
            createShape(true);
            drawEdit(toPointerVector(ShapeGeometry));
        }

        /** @brief Function to add a line to the ShapeGeometry vector.*/
        auto addLineToShapeGeometry(Base::Vector3d p1, Base::Vector3d p2, bool constructionMode)
        {
            auto line = std::make_unique<Part::GeomLineSegment>();
            line->setPoints(p1, p2);
            //Sketcher::GeometryFacade::setConstruction(line.get(), constructionMode);
            return static_cast<Part::GeomLineSegment*>(ShapeGeometry.emplace_back(std::move(line)).get());
        }
        /** @brief Function to add an arc to the ShapeGeometry vector.*/
        auto addArcToShapeGeometry(Base::Vector3d p1, double start, double end, double radius, bool constructionMode)
        {
            auto arc = std::make_unique<Part::GeomArcOfCircle>();
            arc->setCenter(p1);
            arc->setRange(start, end, true);
            arc->setRadius(radius);
            //Sketcher::GeometryFacade::setConstruction(arc.get(), constructionMode);
            return static_cast<Part::GeomArcOfCircle*>(ShapeGeometry.emplace_back(std::move(arc)).get());
        }

        /** @brief Function to add a point to the ShapeGeometry vector.*/
        auto addPointToShapeGeometry(Base::Vector3d p1, bool constructionMode)
        {
            auto point = std::make_unique<Part::GeomPoint>();
            point->setPoint(p1);
            //Sketcher::GeometryFacade::setConstruction(point.get(), constructionMode);
            return static_cast<Part::GeomPoint*>(ShapeGeometry.emplace_back(std::move(point)).get());
        }

        /** @brief Function to add an ellipse to the ShapeGeometry vector.*/
        auto addEllipseToShapeGeometry(
            Base::Vector3d centerPoint,
            Base::Vector3d majorAxisDirection,
            double majorRadius,
            double minorRadius,
            bool constructionMode
        )
        {
            auto ellipse = std::make_unique<Part::GeomEllipse>();
            ellipse->setMajorRadius(majorRadius);
            ellipse->setMinorRadius(minorRadius);
            ellipse->setMajorAxisDir(majorAxisDirection);
            ellipse->setCenter(centerPoint);
            //Sketcher::GeometryFacade::setConstruction(ellipse.get(), constructionMode);
            return static_cast<Part::GeomEllipse*>(ShapeGeometry.emplace_back(std::move(ellipse)).get());
        }

        /** @brief Function to add a circle to the ShapeGeometry vector.*/
        auto addCircleToShapeGeometry(Base::Vector3d centerPoint, double radius, bool constructionMode)
        {
            auto circle = std::make_unique<Part::GeomCircle>();
            circle->setRadius(radius);
            circle->setCenter(centerPoint);
            //Sketcher::GeometryFacade::setConstruction(circle.get(), constructionMode);
            return static_cast<Part::GeomCircle*>(ShapeGeometry.emplace_back(std::move(circle)).get());
        }
    protected:
        using SelectMode = SelectModeT;
        using ModeStateMachine = StateMachine<SelectModeT>;
        using ConstructionMethod = ConstructionMethodT;
        using ConstructionMachine = ConstructionMethodMachine<ConstructionMethodT>;

        std::vector<std::unique_ptr<Part::Geometry>> ShapeGeometry;
        bool continuousMode;
    };
}
